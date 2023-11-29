#include "Peripheral.h"
#include <queue>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "WifiManager.h"
#include "InputParametrs.h"
#define WIRING_CHECK_TIME 600
#define WIRING_BROKEN 1
#define TOF_IRQ_RANGE 1000    // mm
#define FRONT_RESTART_TO 3000 // in ms
#define DEV_RESTART_TO 10
#define SENS_POLL_TIME 30 // sec
constexpr char Daemon_Version[] = "01.00";
int running = 1;
int read_from_ti;
int read_from_dynamic;
int write_to_nrf;
int read_from_nrf;
int write_to_ti;
pthread_t nrf, ti, dynamic;
extern char dev_id[16];
std::string NRF_SW;
std::string NRF_HW;
std::string TI_SW;
std::string TI_HW;
// function responsible for joining threads and closing fds
void cleanup()
{
    running = 0;
    close(read_from_nrf);
    close(write_to_nrf);
    pthread_join(nrf, nullptr);
    close(read_from_ti);
    close(write_to_ti);
    pthread_join(ti, nullptr);
    close(read_from_dynamic);
    pthread_join(dynamic, nullptr);

    syslog(LOG_INFO, "Stopped Hvac\0");
    closelog();
}
/// <summary>
/// thread for time related system calls
/// </summary>
/// <param name="a">-file descriptor of pipe that is connected to main</param>
/// <returns>at exit</returns>
void *dynamic_thrd(void *a)
{
    thread_Data dynamic = *(thread_Data *)a;
    uint8_t dummy = 1;
    uint8_t counter{};
    char line[256];
    bool rest_php_pipe = true;
    FILE *fp;
    char getDynamic_php[] = "php getDynamic10.php\0";

    while (running)
    {
        if (!(counter % 2))
        {
            if (fork() == 0) // fork off to set wifi signal
            {
                int fd;
                for (fd = static_cast<int>(sysconf(_SC_OPEN_MAX)); fd > 0; fd--)
                {
                    close(fd);
                }

                /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
                stdin = fopen("/dev/null", "r");
                stdout = fopen("/dev/null", "w+");
                stderr = fopen("/dev/null", "w+");
                WifiManager a;
                a.write_wifi_signal();
                quick_exit(0);
            }
        }
        if (running)
            fp = popen(getDynamic_php, "r");
        if (fp == nullptr)
            continue;
        while (fgets(line, 256, fp) != NULL && running)
        {
            write(dynamic.pipe_out, &line, 256);
        }
        if (running)
            pclose(fp);
        counter++;
    }
    close(dynamic.pipe_out);
    return nullptr;
}
/// <summary>
///< para> thread that handles uart data from nrf and sends it to main</para>
///  as well as sends data from main to nrf via uart
/// </summary>
/// <param name="a">
/// <para>file descriptor of pipe end that is connected to main</para>
/// <para>file descriptor of pipe end that is connected from main</para>
/// </param>
/// <returns>at exit</returns>
void *nrf_uart_thrd(void *a)
{
    // initializing uart

    // This line is dereferencing the void pointer a and casting it to a thread_Data structure.
    // It's assumed that a is a pointer to a thread_Data object, and this line is making a local copy of that data.
    thread_Data nrf = *(thread_Data *)a;

    // An array of four pollfd structures is declared. pollfd is used for monitoring file descriptors for events.
    //       struct pollfd {
    //        int   fd;         /* file descriptor */
    //        short events;     /* requested events */
    //        short revents;    /* returned events */
    //    };
    pollfd fds[4];
    int num_open_fds = 4;

    // This line initializes a time_t variable with the current time.
    // It's used for tracking the last time the sensors were polled.
    time_t last_polled_sens = time(NULL);

    // It attempts to open a serial port specified by the NRF_SERRIAL_PORT
    // constant with read and write permissions and sets the file descriptor for
    // fds[0] to the returned value. The O_NOCTTY and O_NONBLOCK flags
    // are used in opening the serial port.
    fds[0].fd = open(NRF_SERRIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);

    // This line sets the events field of fds[0] to POLLIN,
    //  indicating that it will be monitored for incoming data.
    fds[0].events = POLLIN;

    // A termios structure called tty is declared.
    // It will be used to configure the serial port settings.
    termios tty;

    // This line attempts to get the current terminal I/O settings for the file
    //  descriptor of the opened serial port. If it fails (returns non-zero),
    //  an error message is logged via syslog, and nullptr is returned from the function.
    if (tcgetattr(fds[0].fd, &tty) != 0)
    {
        syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        return nullptr;
    }

    // This function is used to configure the tty structure for the serial
    //  port communication.
    set_tty(&tty);

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    // This line attempts to apply the updated terminal I/O settings to the serial port.
    //  If it fails, an error message is logged, and nullptr is returned.
    if (tcsetattr(fds[0].fd, TCSANOW, &tty) != 0)
    {
        syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        return nullptr;
    }

    // It calls a function configure_pins with the argument NRF_GPIO_4.
    //  If the function returns an error code, an error message is logged,
    //  and nullptr is returned. This likely configures some GPIO pins associated with the nRF device.
    if (configure_pins(NRF_GPIO_4))
    {
        syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        return nullptr;
    }
    if (configure_pins(NRF_GPIO_5))
    {
        ;
        syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        return nullptr;
    }

    // setup pipe fd
    //  A character array str with a size of 36 characters is declared. It's used to store strings, likely file paths.
    char str[36];

    // This line uses the sprintf function to format a string and store it in the
    //  str array. It appears to be constructing a file path using a format string
    //  specified by SW_VAL_PATH and the value of NRF_GPIO_4.
    //  The resulting string is stored in str.
    sprintf(str, SW_VAL_PATH, NRF_GPIO_4);

    // This line sets the fd field of the fds[1] structure to the value of
    // nrf.pipe_in. This indicates that fds[1] will be monitoring the file
    //  descriptor nrf.pipe_in
    //! where is the nrf.pipe_in defined!?
    fds[1].fd = nrf.pipe_in;

    // The events field of fds[1] is set to POLLIN,
    // indicating that it will be monitored for incoming data.
    fds[1].events = POLLIN;

    // It attempts to open a file specified by the string str in read-only
    //  mode and with non-blocking behavior. The resulting file descriptor is set in fds[2].
    fds[2].fd = open(str, O_RDONLY | O_NONBLOCK);

    // The events field of fds[2] is set to POLL_GPIO, suggesting that it will be monitored for GPIO-related events.
    fds[2].events = POLL_GPIO;

    // Similar to the first sprintf call, this line constructs a file path based
    // on NRF_GPIO_5 and stores it in the str array.
    sprintf(str, SW_VAL_PATH, NRF_GPIO_5);

    // by the updated str in read-only mode and with non-blocking behavior.
    //  The resulting file descriptor is set in fds[3].
    fds[3].fd = open(str, O_RDONLY | O_NONBLOCK);

    //  The events field of fds[3] is set to POLL_GPIO.
    fds[3].events = POLL_GPIO;

    // This loop iterates through the fds array and sets the revents field
    //  of each fds structure to 0. This initializes the revents field
    //  for each file descriptor to zero before monitoring for events.
    for (int p = 0; p < 4; p++)
    {
        fds[p].revents = 0;
    }

    // Three arrays of uint8_t (8-bit unsigned integers) are declared,
    //  each with a size of 32 bytes. These arrays will be used
    //  to store data to be sent to the nRF device.
    uint8_t sens_tx_buff[32];
    uint8_t tof_tx_buff[32];
    uint8_t dev_info[32];

    // A structure SIO_Packet_t named tx_packet is declared.
    //  It appears to be used for packaging data to be transmitted via UART.
    SIO_Packet_t tx_packet;

    // Two integer variables are declared to store the sizes of
    // the data to be transmitted for sensors and TOF (Time of Flight) devices.
    int sens_tx_buff_size;
    int tof_tx_buff_size;

    // This line sets the PacketSrc field of tx_packet to UART_Packet,
    // indicating that the data will be sent via UART.
    tx_packet.PacketSrc = UART_Packet; // 0x01
    tx_packet.CMD = GetInfo;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;

    // An integer variable dev_buff_size is declared.
    int dev_buff_size;

    // It calls a function Set_SIO_TxPacket with the dev_info array and the
    // tx_packet structure. This function likely packages the data and the
    //  packet information into the dev_info array and returns the size
    //  of the data in dev_buff_size.
    dev_buff_size = Set_SIO_TxPacket(dev_info, tx_packet);

    // The write function is used to send the data in the dev_info array to the UART port specified by fds[0].fd.
    write(fds[0].fd, dev_info, dev_buff_size);

    // Get Sensors
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetSensors;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    sens_tx_buff_size = Set_SIO_TxPacket(sens_tx_buff, tx_packet);

    // Get TOF
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetTOF;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    tof_tx_buff_size = Set_SIO_TxPacket(tof_tx_buff, tx_packet);

    //  These lines declare variables for reading and storing data from file
    //  descriptors and for storing the result of the poll function.
    // s is used for the size of read data,
    // ready is used to store the result of the poll call, and
    // buf is a buffer for reading data.
    ssize_t s;
    int ready;
    uint8_t buf[256];

    // A boolean variable wait_for_response is initialized to true.
    // It is used to control whether the system is expecting a response from the nRF device.
    bool wait_for_response = true;

    // The code enters a while loop that continues while the num_open_fds is non-zero.
    //  This loop appears to manage the monitoring and handling of multiple file descriptors.
    while (num_open_fds)
    {
        // The poll function is called to monitor the file descriptors specified in the fds array.
        //  It waits for events on these file descriptors for a duration determined by SENS_POLL_TIME
        //  (possibly in seconds) multiplied by 1000 (to convert to milliseconds)
        //  and then adding 1500 milliseconds. The result is stored in the ready variable.
        ready = poll(fds, 4, SENS_POLL_TIME * 1000 + 1500); // SENS_POLL_TIME: 30 sec

        // This condition checks if the poll function did not encounter an error.
        if (ready != -1)
        {
            if (ready > 0)
            {

                //  it checks if there is data available to read (POLLIN event)
                // and reads the data from this file descriptor,
                //  writing it to nrf.pipe_out. It then sets
                if (fds[0].revents & POLLIN)
                {
                    s = read(fds[0].fd, buf, sizeof(buf));
                    write(nrf.pipe_out, buf, s);
                    wait_for_response = false;
                }

                // If an error event occurs on fds[0] (POLLERRVAL),
                // it logs an error message and closes the file descriptor.
                // The function also reduces the count of open file descriptors,
                // and nullptr is returned (possibly indicating an error).
                else if (fds[0].revents & POLLERRVAL)
                {
                    fds[0].revents = 0;
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 0 }]}\n");
                    close(fds[0].fd); // system needs to be restarted
                    num_open_fds--;
                    return nullptr;
                }

                if (fds[1].revents & POLLIN)
                {
                    if (!wait_for_response)
                    {
                        // data from main to nrf device
                        s = read(fds[1].fd, buf, sizeof(buf));
                        write(fds[0].fd, buf, s);
                        wait_for_response = true;
                    }
                }
                else if (fds[1].revents & POLLERRVAL)
                {
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 1 }]}\n");
                    close(fds[1].fd); // system needs to be restarted
                    fds[1].revents = 0;
                    num_open_fds--;
                    return nullptr;
                }
                if (fds[2].revents & POLLPRI)
                {
                    if (!wait_for_response)
                    {
                        lseek(fds[2].fd, 0, SEEK_SET);
                        s = read(fds[2].fd, buf, sizeof(buf));
                        fds[2].revents = 0;
                        if (s == 2)
                        {
                            if (buf[0] == '0')
                            {
                                write(fds[0].fd, sens_tx_buff, sens_tx_buff_size);
                                wait_for_response = true;
                            }
                        }
                    }
                }
                else if (fds[2].revents & POLLERR)
                {
                    fds[2].revents = 0;
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 2 }]}\n");
                    close(fds[2].fd); // system needs to be restarted
                    num_open_fds--;
                    return nullptr;
                }
                if (fds[3].revents & POLLPRI)
                {
                    if (!wait_for_response)
                    {
                        lseek(fds[3].fd, 0, SEEK_SET);
                        s = read(fds[3].fd, buf, sizeof(buf));
                        fds[3].revents = 0;
                        if (s == 2)
                        {
                            if (buf[0] == '0')
                            {
                                write(fds[0].fd, tof_tx_buff, tof_tx_buff_size);
                                wait_for_response = true;
                            }
                        }
                    }
                }
                else if (fds[3].revents & POLLERR)
                {
                    fds[3].revents = 0;
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 3 }]}\n");
                    close(fds[3].fd); // system needs to be restarted
                    num_open_fds--;
                    return nullptr;
                }
            }

            // If ready is zero or the time since the last sensor polling exceeds
            //  SENS_POLL_TIME (in seconds), and wait_for_response is false,
            //  it sends a data packet (sens_tx_buff) to fds[0] to request sensor information.
            //  The time of the last sensor poll is updated, and wait_for_response is set to true.
            if ((ready == 0 || difftime(time(NULL), last_polled_sens) >= SENS_POLL_TIME) && !wait_for_response)
            {
                write(fds[0].fd, sens_tx_buff, sens_tx_buff_size);
                last_polled_sens = time(NULL);
                wait_for_response = true;
            }
        }
        else
        {
            // Finally, if there is an error (ready is -1) when calling poll,
            //  error messages are logged using syslog, possibly indicating issues with file descriptor monitoring.
            syslog(LOG_INFO, "NRF thrd error -1\n");
            syslog(LOG_INFO, "%s", strerror(errno));
        }
    }

    return nullptr;
}
/// <summary>
/// <para>thread that handles uart data from ti and sends it to main,</para>
/// <para> sends data from main to ti via uart,</para>
/// as well as handles rf commuincation with external sensors
/// </summary>
/// <param name="a">
/// <para>file descriptor of pipe end that is connected to main</para>
/// <para>file descriptor of pipe end that is connected from main</para>
/// </param>
/// <returns>at exit</returns>
void *ti_uart_thrd(void *a)
{
    thread_Data ti = *(thread_Data *)a;
    pollfd fds[3];
    int num_open_fds = 3;
    fds[0].fd = open(TI_SERRIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    fds[0].events = POLLIN;
    termios tty;
    if (tcgetattr(fds[0].fd, &tty) != 0)
    {

        return nullptr;
    }
    set_tty(&tty);
    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    Serial_RxData_t rx_data;
    SIO_Packet_t tx_packet, rx_packet_ti;
    // Save tty settings, also checking for error
    if (tcsetattr(fds[0].fd, TCSANOW, &tty) != 0)
    {

        return nullptr;
    }

    // setup pipe
    fds[1].fd = ti.pipe_in;
    fds[1].events = POLLIN;
    AQ_TH_PR_thld throlds_aq;
    Response_Time Rtv;
    uint8_t cpIndex = 0;
    uint8_t rf_tx_buff[256];
    uint8_t rf_packet[128];
    std::vector<uint32_t> paired_list;
    std::vector<uint8_t> paired_list_rep_counter;
    uint8_t seq_num;
    uint8_t dev_type;
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetInfo;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    int rf_tx_buff_size;
    uint8_t rf_len;
    uint32_t brdcst_addr = 0xffffffff;
    bool aquired_addr = false;
    rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
    write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
    int indx_rev = 0;
    while (num_open_fds)
    {
        int ready;
        ready = poll(fds, 2, 10500);
        if (ready != -1) //
        {
            if (ready > 0)
            {

                uint8_t buf[256];
                if (fds[0].revents & POLLIN)
                {
                    ssize_t s = read(fds[0].fd, buf, sizeof(buf));
                    for (int i = 0; i < s; i++)
                    {
                        if (SerialDataRx(buf[i], &rx_data))
                        {
                            break;
                        }
                    }
                    rx_packet_ti.CMD = (SIO_Cmd_t)rx_data.RxDataArray[CMD_Offset];
                    rx_packet_ti.ACK = rx_data.RxDataArray[ACK_Offset];
                    rx_packet_ti.SID = rx_data.RxDataArray[SID_Offset];
                    uint8_t PayloadLen = rx_data.RxDataLen - PacketMinLength;
                    rx_packet_ti.DataLen = PayloadLen;
                    if (PayloadLen > 0)
                    {
                        memcpy(&rx_packet_ti.DataArray[0], &rx_data.RxDataArray[DATA_Offset], PayloadLen);
                    }
                    rx_packet_ti.CRC = (uint16_t)rx_data.RxDataArray[DATA_Offset + PayloadLen];
                    rx_packet_ti.CRC |= (uint16_t)(rx_data.RxDataArray[DATA_Offset + PayloadLen + 1] << 8);
                    uint16_t inc_crc_ti = crc16(rx_packet_ti.DataArray, rx_packet_ti.DataLen);
                    rx_packet_ti.PacketSrc = UART_Packet;
                    memset(&rx_data, 0, sizeof(rx_data));
                    if (inc_crc_ti == rx_packet_ti.CRC)
                    {
                        switch (rx_packet_ti.CMD)
                        {
                        case Get_packets:
                            if (!aquired_addr)
                                break;
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = Send_packet;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            if (std::find(paired_list.begin(), paired_list.end(), *(uint32_t *)(rx_packet_ti.DataArray + 4)) != paired_list.end())
                            {
                                memcpy(rf_packet, &rx_packet_ti.DataArray[4], 4);
                                seq_num = rx_packet_ti.DataArray[8];
                                seq_num++;
                                memcpy(rf_packet + 8, &seq_num, 1);
                                dev_type = Main_dev;
                                memcpy(rf_packet + 9, &dev_type, 1); // Dev_type main
                                buf[0] = rx_packet_ti.CMD;
                                buf[1] = rx_packet_ti.ACK;
                                buf[3] = rx_packet_ti.DataArray[9];
                                buf[4] = 1;
                                memcpy(buf + 5, (rx_packet_ti.DataArray + 4), 4);
                                switch (rx_packet_ti.DataArray[9])
                                {
                                case AQ_TH_PR:
                                    memcpy(rf_packet + 10, &Rtv, sizeof(Rtv));
                                    rf_len = 10 + sizeof(Rtv);
                                    memcpy(rf_packet + rf_len, &throlds_aq, AQS_THRSHLD_SIZE);

                                    memcpy(buf + 9, &rx_packet_ti.DataArray[10], AQS_DATA_SIZE);
                                    buf[2] = AQS_DATA_SIZE + 6;
                                    rf_len += AQS_THRSHLD_SIZE;
                                default:
                                    break;
                                }
                                memcpy(tx_packet.DataArray, rf_packet, rf_len);
                                cpIndex = rf_len;
                                tx_packet.DataLen = cpIndex;

                                rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                                write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);

                                if (memcmp(rx_packet_ti.DataArray, &brdcst_addr, 4))
                                    write(ti.pipe_out, buf, buf[2] + 3);
                            }
                            else if (!memcmp(rx_packet_ti.DataArray, rf_packet + 4, 4))
                            {
                                memcpy(rf_packet, &rx_packet_ti.DataArray[4], 4);
                                seq_num = rx_packet_ti.DataArray[8];
                                seq_num++;
                                memcpy(rf_packet + 8, &seq_num, 1);
                                dev_type = NO_TYPE;                  // indicate to unpair
                                memcpy(rf_packet + 9, &dev_type, 1); // Dev_type main
                                rf_len = 10;
                                memcpy(tx_packet.DataArray, rf_packet, rf_len);
                                cpIndex = rf_len;
                                tx_packet.DataLen = cpIndex;
                                rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                                write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            }
                            else if (!memcmp(rx_packet_ti.DataArray, &brdcst_addr, 4))
                            {

                                buf[0] = rx_packet_ti.CMD;
                                buf[1] = rx_packet_ti.ACK;
                                buf[2] = 6;
                                buf[3] = rx_packet_ti.DataArray[9];
                                buf[4] = 0;
                                memcpy(buf + 5, &rx_packet_ti.DataArray[4], 4);
                                write(ti.pipe_out, buf, 9);
                            }
                            break;
                        case GetRelaySensor:
                        case Check_Wiring:
                        case SetRelay:
                        case Send_packet:
                            buf[0] = rx_packet_ti.CMD;
                            buf[1] = rx_packet_ti.ACK;
                            buf[2] = rx_packet_ti.DataLen;
                            memcpy(buf + 3, &rx_packet_ti.DataArray[0], rx_packet_ti.DataLen);
                            write(ti.pipe_out, buf, rx_packet_ti.DataLen + 3);
                            break;
                        case GetInfo:
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = Get_addr;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            indx_rev = 0;
                            for (; rx_packet_ti.DataArray[indx_rev] != 0 && indx_rev < sizeof(rx_packet_ti.DataArray); indx_rev++)
                            {
                                TI_HW.push_back(static_cast<char>(rx_packet_ti.DataArray[indx_rev]));
                            }
                            ++indx_rev;
                            for (; rx_packet_ti.DataArray[indx_rev] != 0 && indx_rev < sizeof(rx_packet_ti.DataArray); indx_rev++)
                            {
                                TI_SW.push_back(static_cast<char>(rx_packet_ti.DataArray[indx_rev]));
                            }
                            syslog(LOG_INFO, "TI:: HW:%s SW:%s\n", TI_HW.c_str(), TI_SW.c_str());
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            break;
                        case Get_addr:
                            memcpy(rf_packet + 4, &rx_packet_ti.DataArray[0], 4);
                            aquired_addr = true;
                            buf[0] = rx_packet_ti.CMD;
                            buf[1] = rx_packet_ti.ACK;
                            buf[2] = rx_packet_ti.DataLen;
                            memcpy(buf + 3, &rx_packet_ti.DataArray[0], rx_packet_ti.DataLen);
                            write(ti.pipe_out, buf, rx_packet_ti.DataLen + 3);
                            break;
                        case GET_DEV_ID:
                            syslog(LOG_INFO, "GET_DEV_ID\n");
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = GET_DEV_ID;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;

                            if ((sizeof(dev_id) + TI_HW.length() + 1 + TI_SW.length() + 1 + NRF_HW.length() + 1 + NRF_SW.length() + 1 + sizeof(Daemon_Version)) > sizeof(tx_packet.DataArray))
                            {
                                syslog(LOG_INFO, "ERROR VERSION LENGTH\n");
                                break;
                            }

                            memcpy(tx_packet.DataArray, dev_id, sizeof(dev_id));
                            tx_packet.DataLen += sizeof(dev_id);
                            memcpy(tx_packet.DataArray + tx_packet.DataLen, NRF_HW.c_str(), NRF_HW.length() + 1);
                            tx_packet.DataLen += NRF_HW.length() + 1;
                            memcpy(tx_packet.DataArray + tx_packet.DataLen, NRF_SW.c_str(), NRF_SW.length() + 1);
                            tx_packet.DataLen += NRF_SW.length() + 1;
                            memcpy(tx_packet.DataArray + tx_packet.DataLen, Daemon_Version, sizeof(Daemon_Version));
                            tx_packet.DataLen += sizeof(Daemon_Version);
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            tx_packet.DataLen = 0;
                            break;
                        case feed_wtd:

                            break;
                        default:
                            syslog(LOG_INFO, "ERROR_CMD %d\n", rx_packet_ti.CMD);
                            break;
                        }
                    }
                    else
                    {
                        tx_packet.PacketSrc = UART_Packet;
                        tx_packet.CMD = rx_packet_ti.CMD;
                        tx_packet.ACK = ERROR_CRC;
                        tx_packet.SID = 0x01;
                        tx_packet.DataLen = 0;
                        rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                        write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                    }
                    // thread to main pipe
                }
                else if (fds[0].revents & POLLERRVAL)
                {
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"ti_pipe\": 0 }]}\n");
                    close(fds[0].fd); // system needs to be restarted
                    num_open_fds--;
                }
                if (fds[1].revents & POLLIN)
                {
                    ssize_t s = read(fds[1].fd, buf, sizeof(buf)); // main to thread pipe
                    int indx_cm = 0;
                    // init
                    while (indx_cm < s)
                    {
                        switch (buf[indx_cm])
                        {
                        case Set_limits:
                            switch (buf[indx_cm + 1])
                            {
                            case AQ_TH_PR:
                                memcpy(&throlds_aq, buf + indx_cm + 2, AQS_THRSHLD_SIZE);
                                indx_cm += (2 + AQS_THRSHLD_SIZE);
                            default:
                                break;
                            }
                            break;
                        case Set_time:
                            switch (buf[indx_cm + 1])
                            {
                            case AQ_TH_PR:
                                memcpy(&Rtv, buf + indx_cm + 2, sizeof(Rtv));
                                indx_cm += 2 + sizeof(Rtv);
                            default:
                                break;
                            }
                            break;
                        case Set_paired:
                            paired_list.clear();
                            paired_list_rep_counter.clear();
                            for (int i = 0; i < buf[indx_cm + 1]; i++)
                            {
                                paired_list.push_back(*(uint32_t *)(buf + indx_cm + 2 + 4 * i));
                                paired_list_rep_counter.push_back(0);
                            }
                            indx_cm += (2 + buf[indx_cm + 1] * 4);
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = StartPairing;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            break;
                        case Get_wiring: // guaranteed to be the only wiring related cmd in a packet from main
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = GetRelaySensor;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            indx_cm += 1;
                            break;
                        case Wiring_check: // guaranteed to be the only wiring related cmd in a packet from main
                            tx_packet.PacketSrc = UART_Packet;
                            syslog(LOG_INFO, "DO: Wiring_check \n");
                            tx_packet.CMD = Check_Wiring;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            indx_cm += 1;
                            break;
                        case Set_relays: // guaranteed to be the only wiring related cmd in a packet from main
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = SetRelay; //ToFind
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            if ((static_cast<long>(indx_cm) + 1 + RELAY_OUT_CNT > s) ||
                                ((static_cast<unsigned long>(indx_cm) + 1 + RELAY_OUT_CNT) > sizeof(buf)))
                            {
                                syslog(LOG_INFO, "ERROR: SetRelay pipe_data read_size: %d, buf_size %d, index: %d\n", s, sizeof(buf), (static_cast<unsigned long>(indx_cm) + 1 + RELAY_OUT_CNT));
                                indx_cm += (1 + RELAY_OUT_CNT);
                                break;
                            }
                            memcpy(tx_packet.DataArray, buf + indx_cm + 1, RELAY_OUT_CNT);
                            tx_packet.DataLen = RELAY_OUT_CNT;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            syslog(LOG_INFO, "DO: SetRelay [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d] \n", buf[indx_cm + 1], buf[indx_cm + 2], buf[indx_cm + 3], buf[indx_cm + 4], buf[indx_cm + 5], buf[indx_cm + 6], buf[indx_cm + 7], buf[indx_cm + 8], buf[indx_cm + 9], buf[indx_cm + 10]);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            indx_cm += (1 + RELAY_OUT_CNT);
                            break;
                        default:
                            break;
                        }
                    }
                }
                else if ((fds[1].revents & POLLERRVAL))
                {
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"ti_pipe\": 1 }]}\n");
                    close(fds[1].fd); // system needs to be restarted
                    fds[1].revents = 0;
                    num_open_fds--;
                    return nullptr;
                }
            }
            else
            {
                tx_packet.PacketSrc = UART_Packet;
                tx_packet.CMD = feed_wtd;
                tx_packet.ACK = ERROR_NO;
                tx_packet.SID = 0x01;
                tx_packet.DataLen = 0;
                rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                // write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
            }
        }
    }

    return nullptr;
}
/// <summary>
/// send key event to display
/// </summary>
/// <param name="a">
/// -character of the key for which event is to be sent
/// </param>
/// <returns></returns>
void key_event(char a)
{
    if (fork() == 0)
    {
        char str[36];
        sprintf(str, SYS_KEY_EVNT, a);
        system("export DISPLAY=:0");
        system(str);
        quick_exit(0);
    }
}
int main(int argc, char *argv[])
{
    daemonize();
    std::atexit(cleanup);
    openlog(argv[0], LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started %s Version %s", argv[0], Daemon_Version);
    chdir(WEB_DIR);
    if (!get_device_id()) // init device id from linux
    {
        syslog(LOG_INFO, "Error: get_device_id \n");
    }
    device_t main_dev;
    main_dev.address = 0xffffffff; // this address is used in rf comms
    main_dev.paired = true;
    main_dev.type = Main_dev;
    Response_Time Rtv;
    Rtv.TP_internal_sesn_poll = 200; // 2sec
    Rtv.TT_if_ack = 40;              // 10 min
    Rtv.TT_if_nack = 25;
    AQ_TH_PR_thld throlds_aq;
    AQ_TH_PR_vals aqthpr_dum_val;
    uint8_t cpIndex = 0;
    rgb_vals RGBm, RGBm_last;
    uint8_t buf[1024];
    int dev_count_indx;
    std::vector<device_t> device_list;
    std::vector<config_time> time_configs;
    std::vector<config_thresholds> throlds;
    std::vector<uint8_t> relays_in_l, relays_in, wiring_state;
    if (get_Config_list(time_configs))
    {
        for (auto i : time_configs)
        {
            switch (i.ext_sens_type)
            {
            case AQ_TH_PR:
                Rtv.TP_internal_sesn_poll = i.TP_internal_sesn_poll;
                Rtv.TT_if_ack = i.TT_if_ack;
                Rtv.TT_if_nack = i.TT_if_nack;
                break;
            default:
                break;
            }
        }
    }
    else
    {
        syslog(LOG_INFO, "Error: get_Config_list \n");
    }
    if (!get_paired_list(device_list)) // send to ti
    {
        syslog(LOG_INFO, "Error: get_paired \n");
    }
    if (get_thresholds_list(throlds))
    {
        for (auto i : throlds)
        {
            switch (i.sens_type)
            {
            case SNS_temperature:
                if (i.max_alert_value > 127)
                    i.max_alert_value = 127;
                throlds_aq.temp_high = static_cast<uint8_t>(i.max_alert_value);
                if (i.min_alert_value < -128)
                    i.min_alert_value = -128;
                throlds_aq.temp_low = static_cast<uint8_t>(i.min_alert_value);
                break;
            case SNS_humidity:
                if (i.max_alert_value > 100)
                    i.max_alert_value = 100;
                throlds_aq.humidity_high = static_cast<uint8_t>(i.max_alert_value);
                if (i.min_alert_value < 0)
                    i.min_alert_value = 0;
                throlds_aq.humidity_low = static_cast<uint8_t>(i.min_alert_value);
                break;
            case SNS_co2:
                throlds_aq.c02_high = i.max_alert_value;
                break;
            case SNS_etoh:
                if (i.max_alert_value > 127)
                    i.max_alert_value = 127;
                throlds_aq.etoh_high = static_cast<uint8_t>(i.max_alert_value);
                break;
            case SNS_iaq:
                if (i.max_alert_value > 5)
                    i.max_alert_value = 5;
                throlds_aq.iaq_high = static_cast<uint8_t>(i.max_alert_value);
                break;
            case SNS_Tvoc:
                if (i.max_alert_value > 127)
                    i.max_alert_value = 127;
                throlds_aq.Tvoc_high = static_cast<uint8_t>(i.max_alert_value);
                break;
            case SNS_pressure:
                throlds_aq.pressure_high = i.max_alert_value;
                break;
            default:
                break;
            }
        }
    }
    else
    {
        syslog(LOG_INFO, "Error: get_thresholds_list \n");
    }
    uint8_t Thread_buff[256];
    SIO_Packet_t tx_packet, rx_packet;
    Serial_RxData_t packet_rx_settings;
    int Thread_send_size;
    int16_t nrf_Temp;
    uint8_t nrf_Hum;
    uint16_t nrf_aqs_CO2eq;
    uint16_t nrf_aqs_etoh;
    uint16_t nrf_aqs_TVOC;
    uint8_t nrf_aqs_iaq;
    uint16_t nrf_pressure;
    uint8_t brighness_mode = 1;
    /// <summary>
    /// threading
    /// create 2 threads with 2 pipes each
    /// use poll for handling read write
    /// </summary>
    thread_Data nrf_vals, ti_vals, dynamic_vals;
    bool wait_for_wiring_check = true;
    ;
    int pipe_nrf_write_fd[2];
    pipe2(pipe_nrf_write_fd, O_NONBLOCK);
    int pipe_nrf_read_fd[2];
    pipe2(pipe_nrf_read_fd, O_NONBLOCK);

    int pipe_dynamic_fd[2];
    pipe2(pipe_dynamic_fd, O_NONBLOCK);
    dynamic_vals.pipe_out = pipe_dynamic_fd[1];
    read_from_dynamic = pipe_dynamic_fd[0];
    pthread_create(&dynamic, nullptr, dynamic_thrd, (void *)&dynamic_vals);

    nrf_vals.pipe_out = pipe_nrf_read_fd[1];
    nrf_vals.pipe_in = pipe_nrf_write_fd[0];
    read_from_nrf = pipe_nrf_read_fd[0];
    write_to_nrf = pipe_nrf_write_fd[1];
    pthread_create(&nrf, nullptr, nrf_uart_thrd, (void *)&nrf_vals);

    int pipe_ti_write_fd[2];
    pipe2(pipe_ti_write_fd, O_NONBLOCK);
    int pipe_ti_read_fd[2];
    pipe2(pipe_ti_read_fd, O_NONBLOCK);
    ti_vals.pipe_out = pipe_ti_read_fd[1];
    ti_vals.pipe_in = pipe_ti_write_fd[0];
    read_from_ti = pipe_ti_read_fd[0];
    write_to_ti = pipe_ti_write_fd[1];
    pthread_create(&ti, nullptr, ti_uart_thrd, (void *)&ti_vals);
    pollfd pipe_fds[3];
    pipe_fds[0].fd = read_from_nrf;
    pipe_fds[0].events = POLLIN;
    pipe_fds[1].fd = read_from_ti;
    pipe_fds[1].events = POLLIN;
    pipe_fds[2].fd = read_from_dynamic;
    pipe_fds[2].events = POLLIN;
    int num_open_fds = 3;
    RGBm.red = 255;
    RGBm.blue = 255;
    RGBm.green = 255;
    RGBm.mode = LED_FADE;
    uint16_t RangeMilliMeter;
    uint16_t fan_speed;
    uint32_t Luminosity;
    bool pairing = false;
    bool last_pairing = false;
    bool wiring_check_f = false;
    int tmr_cntr = WIRING_CHECK_TIME;
    uint64_t last_update_tick{};
    uint64_t last_update_tick_l{};
    int error_dynamic_counter{};
    // set initial configs
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = InitMcus;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.temp_high, sizeof(throlds_aq.temp_high));
    cpIndex += sizeof(throlds_aq.temp_high);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.temp_low, sizeof(throlds_aq.temp_low));
    cpIndex += sizeof(throlds_aq.temp_low);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.humidity_high, sizeof(throlds_aq.humidity_high));
    cpIndex += sizeof(throlds_aq.humidity_high);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.humidity_low, sizeof(throlds_aq.humidity_low));
    cpIndex += sizeof(throlds_aq.humidity_low);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.pressure_high, sizeof(throlds_aq.pressure_high));
    cpIndex += sizeof(throlds_aq.pressure_high);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.c02_high, sizeof(throlds_aq.c02_high));
    cpIndex += sizeof(throlds_aq.c02_high);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.Tvoc_high, sizeof(throlds_aq.Tvoc_high));
    cpIndex += sizeof(throlds_aq.Tvoc_high);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.etoh_high, sizeof(throlds_aq.etoh_high));
    cpIndex += sizeof(throlds_aq.etoh_high);

    memcpy(tx_packet.DataArray + cpIndex, &throlds_aq.iaq_high, sizeof(throlds_aq.iaq_high));
    cpIndex += sizeof(throlds_aq.iaq_high);
    tx_packet.DataLen = cpIndex;
    Thread_send_size = Set_SIO_TxPacket(Thread_buff, tx_packet);
    write(write_to_nrf, Thread_buff, Thread_send_size);
    // AQ_TH_PR
    buf[0] = Set_limits;
    buf[1] = AQ_TH_PR;
    memcpy(buf + 2, &throlds_aq, AQS_THRSHLD_SIZE);
    cpIndex = 2 + AQS_THRSHLD_SIZE;
    buf[cpIndex++] = Set_paired;
    dev_count_indx = cpIndex;
    buf[dev_count_indx] = 0;
    cpIndex++;
    syslog(LOG_INFO, "paired: ");
    for (auto i : device_list)
    {
        if (i.paired)
        {
            memcpy(buf + cpIndex, &i.address, sizeof(i.address));
            cpIndex += sizeof(i.address);
            syslog(LOG_INFO, "%x ", i.address);
            buf[dev_count_indx]++;
        }
    }
    syslog(LOG_INFO, "\n");
    if (buf[dev_count_indx] == 0)
    {
        cpIndex -= 2;
    }
    buf[cpIndex++] = Set_time;
    buf[cpIndex++] = AQ_TH_PR;
    memcpy(buf + cpIndex, &Rtv, sizeof(Rtv));
    syslog(LOG_INFO, "tack: %d tnack : %d senspt : %d\n", Rtv.TT_if_ack, Rtv.TT_if_nack, Rtv.TP_internal_sesn_poll);
    cpIndex += sizeof(Rtv);
    bool wait_resp = false;
    bool check_paired_list = false;
    write(write_to_ti, buf, cpIndex);
    while (num_open_fds && running)
    {
        int result;

        result = poll(pipe_fds, 3, 60000);
        if (result != -1)
        {

            if (result > 0)
            {
                // pipe event to handle data from nrf
                if (pipe_fds[0].revents & POLLIN)
                {
                    ssize_t s = read(pipe_fds[0].fd, buf, sizeof(buf));
                    for (int i = 0; i < s; i++)
                    {
                        if (SerialDataRx(buf[i], &packet_rx_settings))
                        {
                            break;
                        }
                    }
                    rx_packet.CMD = (SIO_Cmd_t)packet_rx_settings.RxDataArray[CMD_Offset];
                    rx_packet.ACK = packet_rx_settings.RxDataArray[ACK_Offset];
                    rx_packet.SID = packet_rx_settings.RxDataArray[SID_Offset];
                    uint8_t PayloadLen = packet_rx_settings.RxDataLen - PacketMinLength;
                    rx_packet.DataLen = PayloadLen;
                    if (PayloadLen > 0)
                    {
                        memcpy(&rx_packet.DataArray[0], &packet_rx_settings.RxDataArray[DATA_Offset], PayloadLen);
                    }
                    rx_packet.CRC = (uint16_t)packet_rx_settings.RxDataArray[DATA_Offset + PayloadLen];
                    rx_packet.CRC |= (uint16_t)(packet_rx_settings.RxDataArray[DATA_Offset + PayloadLen + 1] << 8);
                    uint16_t inc_crc_nrf = crc16(rx_packet.DataArray, rx_packet.DataLen);
                    rx_packet.PacketSrc = UART_Packet;
                    memset(&packet_rx_settings, 0, sizeof(packet_rx_settings));
                    int indx_rev = 0;
                    // check data integridy
                    if (inc_crc_nrf == rx_packet.CRC)
                    {
                        if (rx_packet.ACK != ERROR_NO)
                        {
                            syslog(LOG_INFO, "cmd:%d ack:%d\n", rx_packet.CMD, rx_packet.ACK);
                        }
                        else
                        {
                            switch (rx_packet.CMD)
                            {
                            case InitMcus:
                                break;
                            case GetInfo:
                                indx_rev = 0;
                                for (; rx_packet.DataArray[indx_rev] != 0 && indx_rev < sizeof(rx_packet.DataArray); indx_rev++)
                                {
                                    NRF_HW.push_back(static_cast<char>(rx_packet.DataArray[indx_rev]));
                                }
                                ++indx_rev;
                                for (; rx_packet.DataArray[indx_rev] != 0 && indx_rev < sizeof(rx_packet.DataArray); indx_rev++)
                                {
                                    NRF_SW.push_back(static_cast<char>(rx_packet.DataArray[indx_rev]));
                                }
                                syslog(LOG_INFO, "NRF:: HW:%s SW:%s\n", NRF_HW.c_str(), NRF_SW.c_str());
                                break;
                            case GetTOF:
                                memcpy(&RangeMilliMeter, rx_packet.DataArray, sizeof(RangeMilliMeter));
                                memcpy(&Luminosity, rx_packet.DataArray + sizeof(RangeMilliMeter), sizeof(Luminosity));
                                if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE)
                                {
                                    key_event('n');
                                }
                                if (brighness_mode == 1)
                                {
                                    if (!setBrightness(Luminosity))
                                    {
                                        syslog(LOG_INFO, "Error: setBrightness \n");
                                    }
                                }

                                break;
                            case GetSensors:
                                cpIndex = 0;
                                memcpy(&nrf_Temp, rx_packet.DataArray + cpIndex, sizeof(nrf_Temp));
                                cpIndex += sizeof(nrf_Temp);

                                memcpy(&nrf_Hum, rx_packet.DataArray + cpIndex, sizeof(nrf_Hum));
                                cpIndex += sizeof(nrf_Hum);

                                memcpy(&nrf_aqs_CO2eq, rx_packet.DataArray + cpIndex, sizeof(nrf_aqs_CO2eq));
                                cpIndex += sizeof(nrf_aqs_CO2eq);

                                memcpy(&nrf_aqs_etoh, rx_packet.DataArray + cpIndex, sizeof(nrf_aqs_etoh));
                                cpIndex += sizeof(nrf_aqs_etoh);

                                memcpy(&nrf_aqs_TVOC, rx_packet.DataArray + cpIndex, sizeof(nrf_aqs_TVOC));
                                cpIndex += sizeof(nrf_aqs_TVOC);

                                memcpy(&nrf_aqs_iaq, rx_packet.DataArray + cpIndex, sizeof(nrf_aqs_iaq));
                                cpIndex += sizeof(nrf_aqs_iaq);

                                memcpy(&nrf_pressure, rx_packet.DataArray + cpIndex, sizeof(nrf_pressure));
                                cpIndex += sizeof(nrf_pressure);

                                memcpy(&RangeMilliMeter, rx_packet.DataArray + cpIndex, sizeof(RangeMilliMeter));
                                cpIndex += sizeof(RangeMilliMeter);

                                memcpy(&Luminosity, rx_packet.DataArray + cpIndex, sizeof(Luminosity));
                                cpIndex += sizeof(Luminosity);
                                memcpy(&fan_speed, rx_packet.DataArray + cpIndex, sizeof(fan_speed));
                                cpIndex += sizeof(fan_speed);
                                if (!set_fan_speed_INFO(fan_speed)) // in SetVentilator.php class
                                {
                                    syslog(LOG_INFO, "Error: setFanSpeed\n");
                                }
                                if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE)
                                {
                                    key_event('n');
                                }

                                if (brighness_mode == 1)
                                {
                                    if (!setBrightness(Luminosity))
                                    {
                                        syslog(LOG_INFO, "Error: setBrightness\n");
                                    }
                                }
                                // alert
                                {
                                    bool ret_cb = true;
                                    if (nrf_Temp > throlds_aq.temp_high * 10)
                                    {
                                        ret_cb = setAlert(0, Alert_temp_high, LVL_Emergency);
                                    }
                                    else if (nrf_Temp < throlds_aq.temp_low * 10)
                                    {
                                        ret_cb = setAlert(0, Alert_temp_low, LVL_Emergency);
                                    }
                                    else if (nrf_Hum > throlds_aq.humidity_high)
                                    {
                                        ret_cb = setAlert(0, Alert_humidity_high, LVL_Emergency);
                                    }
                                    else if (nrf_Hum < throlds_aq.humidity_low)
                                    {
                                        ret_cb = setAlert(0, Alert_humidity_low, LVL_Emergency);
                                    }
                                    else if (nrf_pressure > throlds_aq.pressure_high)
                                    {
                                        ret_cb = setAlert(0, Alert_pressure_high, LVL_Emergency);
                                    }
                                    else if (nrf_aqs_CO2eq > throlds_aq.c02_high)
                                    {
                                        ret_cb = setAlert(0, Alert_c02_high, LVL_Emergency);
                                    }
                                    else if (nrf_aqs_TVOC > throlds_aq.Tvoc_high * 1000)
                                    {
                                        ret_cb = setAlert(0, Alert_Tvoc_high, LVL_Emergency);
                                    }
                                    else if (nrf_aqs_etoh > throlds_aq.etoh_high * 100)
                                    {
                                        ret_cb = setAlert(0, Alert_etoh_high, LVL_Emergency);
                                    }
                                    else if (nrf_aqs_iaq > throlds_aq.iaq_high * 10)
                                    {
                                        ret_cb = setAlert(0, Alert_iaq_high, LVL_Emergency);
                                    }
                                    if (!ret_cb)
                                    {
                                        syslog(LOG_INFO, "Error: setAlert \n");
                                    }
                                }
                                if (!setSensorData(main_dev, rx_packet.DataArray, rx_packet.DataLen))
                                {
                                    syslog(LOG_INFO, "Error: setSensorData \n");
                                }
                                break;
                            case SetColorRGB:
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
                else if (pipe_fds[0].revents & POLLERRVAL)
                {
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"pipe\": 0 }]}\n");
                    return 0;
                    close(pipe_fds[0].fd);
                    num_open_fds--;
                }
                // pipe event to handle data from ti
                if (pipe_fds[1].revents & POLLIN)
                {

                    [[maybe_unused]] ssize_t s = read(pipe_fds[1].fd, Thread_buff, sizeof(Thread_buff));

                    wait_resp = false;
                    if (Thread_buff[1] != ERROR_NO)
                    {
                        switch (Thread_buff[0])
                        {
                        case SetRelay:
                            switch (Thread_buff[1])
                            {
                            case ERROR_WIRING_NOT_CONNECTED:
                                wait_for_wiring_check = true;
                                setAlert(0, Alert_wiring_not_connected, LVL_Emergency);
                                syslog(LOG_INFO, "ERROR_WIRING_NOT_CONNECTED\n");
                                syslog(LOG_INFO, "~%d ", Thread_buff[2]);
                                buf[0] = Wiring_check;
                                wait_resp = true;
                                write(write_to_ti, buf, 1); // check wiring
                                break;
                            case ERROR_COULD_NOT_SET_RELAY:
                                syslog(LOG_INFO, "ERROR_COULD_NOT_SET_RELAY\n");
                                if (Thread_buff[2])
                                {
                                    syslog(LOG_INFO, "%d\n relay indx ", Thread_buff[2]);
                                    for (int k = 0; k < Thread_buff[2]; k++)
                                    {
                                        syslog(LOG_INFO, "%d ", Thread_buff[3 + k]);
                                    }
                                    syslog(LOG_INFO, "\n");
                                }

                                break;
                            }
                            break;

                            syslog(LOG_INFO, "\n");
                        default:
                            break;
                        }
                        syslog(LOG_INFO, "cmd:%d ack:%d\n", Thread_buff[0], Thread_buff[1]);
                    }
                    else
                    {
                        device_t inc;
                        switch (Thread_buff[0])
                        {
                        case GetRelaySensor:
                            break;
                        case Check_Wiring:
                            syslog(LOG_INFO, "Ret: Check_Wiring\n");
                            wait_for_wiring_check = false;
                            if (Thread_buff[2] == WIRING_IN_CNT)
                            {

                                wait_for_wiring_check = false;
                                wiring_state.clear();
                                for (int i = 0; i < WIRING_IN_CNT; i++)
                                {
                                    wiring_state.push_back(Thread_buff[3 + i]); // Fill wiring status/ important
                                }
                                if (!setWiring(wiring_state)) // It does nothing  and return true always.
                                {
                                    syslog(LOG_INFO, "Error: setWiring \n");
                                }
                                syslog(LOG_INFO, "W:[%d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n", wiring_state[0], wiring_state[1], wiring_state[2], wiring_state[3], wiring_state[4], wiring_state[5], wiring_state[6], wiring_state[7], wiring_state[8], wiring_state[9]);
                                buf[0] = Set_relays;
                                for (int i = 0; i < RELAY_OUT_CNT; i++)
                                {
                                    if (wiring_state[i] == WIRING_BROKEN && relays_in[i] == 1)
                                    {
                                        setAlert(0, Alert_wiring_not_connected, LVL_Emergency); // Send Alert
                                        syslog(LOG_INFO, "wiring indx %d ", i);
                                        buf[0] = NO_CMD;
                                        relays_in_l = relays_in; // update last relay
                                        break;
                                    }
                                    buf[1 + i] = relays_in[i];
                                }
                                if (buf[0] != NO_CMD)
                                {
                                    wait_resp = true;
                                    write(write_to_ti, buf, 1 + RELAY_OUT_CNT);
                                }
                            }
                            break;
                        case SetRelay:
                            syslog(LOG_INFO, "RET: SetRelay \n");
                            relays_in_l = relays_in;
                            break;
                        case Send_packet:
                            break;
                        case Get_packets:

                            inc.address = *((uint32_t *)(Thread_buff + 5));
                            inc.paired = Thread_buff[4];
                            inc.type = Thread_buff[3];
                            if (inc.paired == pair)
                            {
                                switch (inc.type)
                                {
                                case AQ_TH_PR:

                                    aqthpr_dum_val.Tvoc = Thread_buff[9];
                                    aqthpr_dum_val.etoh = Thread_buff[10];
                                    aqthpr_dum_val.iaq = Thread_buff[11];
                                    aqthpr_dum_val.temp = static_cast<uint16_t>((Thread_buff[13] << 8) | Thread_buff[12]);
                                    aqthpr_dum_val.humidity = Thread_buff[14];
                                    aqthpr_dum_val.c02 = static_cast<uint16_t>((Thread_buff[16] << 8) | Thread_buff[15]);
                                    aqthpr_dum_val.pressure = static_cast<uint16_t>((Thread_buff[18] << 8) | Thread_buff[17]);

                                    // alert
                                    {
                                        bool ret_cb = true;
                                        if (aqthpr_dum_val.temp / 10.0 > throlds_aq.temp_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_temp_high, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.temp / 10.0 < throlds_aq.temp_low)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_temp_low, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.humidity > throlds_aq.humidity_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_humidity_high, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.humidity < throlds_aq.humidity_low)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_humidity_low, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.pressure > throlds_aq.pressure_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_pressure_high, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.c02 > throlds_aq.c02_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_c02_high, LVL_Emergency); // here
                                        }
                                        else if (aqthpr_dum_val.Tvoc > throlds_aq.Tvoc_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_Tvoc_high, LVL_Emergency); // here
                                        }
                                        else if (aqthpr_dum_val.etoh > throlds_aq.etoh_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_etoh_high, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.iaq / 10.0 > throlds_aq.iaq_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_iaq_high, LVL_Emergency); // here
                                        }
                                        else if (!ret_cb)
                                        {
                                            syslog(LOG_INFO, "Error: setAlert \n");
                                        }
                                    }
                                    if (!setSensorData(inc, &Thread_buff[9], AQS_DATA_SIZE))
                                    {
                                        syslog(LOG_INFO, "Error: setSensorData \n");
                                    }
                                default:
                                    break;
                                }
                            }
                            else if (pairing)
                            {

                                if (addPendingSensor(inc))
                                {
                                    syslog(LOG_INFO, "AddPending: %x\n", inc.address);
                                }
                                else
                                    syslog(LOG_INFO, "Error: addPendingSensor \n");
                            }
                            break;
                        case Get_addr:
                            main_dev.address = *(uint32_t *)(Thread_buff + 3);

                            break;
                        default:
                            break;
                        }
                    }
                }
                else if (pipe_fds[1].revents & POLLERRVAL)
                {
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"pipe\": 1 }]}\n");
                    close(pipe_fds[1].fd);
                    num_open_fds--;
                }
                // pipe event to get data from backend
                if (pipe_fds[2].revents & POLLIN)
                {
                    [[maybe_unused]] ssize_t s = read(pipe_fds[2].fd, Thread_buff, sizeof(Thread_buff));

                    if (!get_dynamic1(relays_in, pairing, check_paired_list, wiring_check_f, RGBm, brighness_mode, last_update_tick, Thread_buff, sizeof(Thread_buff)))
                    {
                        // syslog(LOG_INFO, "Error: get_dynamic \n");
                        error_dynamic_counter++;
                        if (error_dynamic_counter > DEV_RESTART_TO)
                        {
                            // syslog(LOG_EMERG, "Error: RESTARTING DEVICE NOW!!!\n");
                            // restart dev
                        }
                        continue;
                    }
                    error_dynamic_counter = 0;
                    if (last_update_tick > last_update_tick_l)
                    {
                        if ((last_update_tick - last_update_tick_l) > FRONT_RESTART_TO)
                        {
                            // restart browser
                        }
                        last_update_tick_l = last_update_tick;
                    }
                    cpIndex = 0;
                    if (pairing != last_pairing || check_paired_list)
                    {
                        if (get_paired_list(device_list))
                        {

                            buf[cpIndex++] = Set_paired;
                            dev_count_indx = cpIndex;
                            buf[dev_count_indx] = 0;
                            cpIndex++;
                            for (auto i : device_list)
                            {
                                if (i.paired)
                                {
                                    memcpy(buf + cpIndex, &i.address, sizeof(i.address));
                                    cpIndex += sizeof(i.address);
                                    buf[dev_count_indx]++;
                                }
                            }
                        }
                        else
                        {
                            syslog(LOG_INFO, "Error: get_paired \n");
                        }
                        last_pairing = pairing;
                    }

                    tmr_cntr++;
                    if (tmr_cntr > WIRING_CHECK_TIME || wiring_check_f) // wiring_check_f call from UI
                    {
                        tmr_cntr = 0;
                        //
                        buf[cpIndex++] = Wiring_check;
                    }
                    else if (relays_in != relays_in_l && !wait_for_wiring_check)
                    {
                        uint8_t last_indx = cpIndex;
                        buf[cpIndex] = Set_relays;
                        for (int i = 0; i < RELAY_OUT_CNT; i++)
                        {
                            if (wiring_state[i] == WIRING_BROKEN && relays_in[i] == 1)
                            {
                                syslog(LOG_INFO, "ERROR_WIRING_NOT_CONNECTED\n");
                                syslog(LOG_INFO, "%d ", i);
                                buf[last_indx] = Wiring_check; // unset cmd
                                cpIndex = last_indx;
                                break;
                            }
                            buf[++cpIndex] = relays_in[i];
                        }
                        cpIndex += 1;
                    }
                    if (!wait_resp && cpIndex)
                    {
                        wait_resp = true;
                        write(write_to_ti, buf, cpIndex);
                    }

                    if (!(RGBm_last == RGBm))
                    {
                        tx_packet.PacketSrc = UART_Packet;
                        tx_packet.CMD = SetColorRGB;
                        tx_packet.ACK = ERROR_NO;
                        tx_packet.SID = 0x01;
                        tx_packet.DataLen = 5;
                        tx_packet.DataArray[0] = RGBm.red;
                        tx_packet.DataArray[1] = RGBm.green;
                        tx_packet.DataArray[2] = RGBm.blue;
                        tx_packet.DataArray[3] = 255;
                        tx_packet.DataArray[4] = RGBm.mode;
                        Thread_send_size = Set_SIO_TxPacket(Thread_buff, tx_packet);
                        syslog(LOG_INFO, "DO::RGB\n (%d,%d,%d) mode:%d\n", RGBm.red, RGBm.green, RGBm.blue, RGBm.mode);
                        write(write_to_nrf, Thread_buff, Thread_send_size);
                        RGBm_last.red = RGBm.red;
                        RGBm_last.green = RGBm.green;
                        RGBm_last.blue = RGBm.blue;
                        RGBm_last.mode = RGBm.mode;
                    }
                }
                else if (pipe_fds[2].revents & POLLERRVAL)
                {
                    syslog(LOG_INFO, "{\"errno\": 12,\"data\":[{\"pipe\": 2 }]}\n");
                    close(pipe_fds[2].fd);
                    num_open_fds--;
                }
            }
        }
    }
    exit(0);
}
