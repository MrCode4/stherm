/**
 * @file main.cpp
 * @brief Main file for the Linux daemon.
 *
 * This file contains the main code for the Linux daemon, which manages the
 * communication between different peripheral devices and the system. It also handles
 * time-related system calls, Wi-Fi signal management, and running PHP scripts.
 */


#include "Peripheral.h"
#include <queue>
#include <thread>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include"WifiManager.h"
#include"InputParametrs.h"
#include <fstream>
#include <algorithm>
#include "DeltaCorrection.h"
#include "NRFThread.h"
#include "Daemon_helper.h"
#include "php_interface.h"
#include "modules/DaemonStatus.h"
// Definitions for constants used throughout the program
#define WIRING_CHECK_TIME 600
#define WIRING_BROKEN 1
//#define TOF_IRQ_RANGE 1000 //mm
#define FRONT_RESTART_TO 3000 //in ms
#define DEV_RESTART_TO 10

#define MIN_BACKLIGHT_PERCENT_ON 5

constexpr char Daemon_Version[] = "01.01";
int running = 1;
int set_update_ti = 0;
int read_from_ti;
int read_from_dynamic;
int write_to_nrf;
int read_from_nrf;
int write_to_ti;
int key_evt_file = 0;
pthread_t nrf, ti, dynamic;
extern char dev_id[16];
std::string NRF_SW;
std::string NRF_HW;
std::string TI_SW;
std::string TI_HW;
int g_sec_counter = 0;
DeltaCorrection delta_correction;


/**
 * @brief count tempreture in celsius to fahrenheit
 * 
 * @param raw_val data in celsius
 * @return int16_t result
 */
int16_t to_fahrenheit(int16_t raw_val)
{
    return 9 * raw_val / 5 + 320;
}

/**
 * @brief count tempreture in fahrenheit to celsius
 * 
 * @param raw_val data in fahrenheit
 * @return int16_t  result
 */
int16_t to_celsius(int16_t raw_val)
{
    return (raw_val - 320) * 5 / 9;
}
/**
 * @brief Function responsible for joining threads and closing file descriptors.
 */

void cleanup()
{
    running = 0;
    close(read_from_nrf);
    close(write_to_nrf);
    pthread_join(nrf, nullptr);
    close(read_from_ti);
    close(write_to_ti);
    pthread_join(ti, nullptr);
    syslog(LOG_EMERG, "Stopped Hvac\0");
    closelog();
    ioctl(key_evt_file, UI_DEV_DESTROY);
    close(key_evt_file);
    kill(0, SIGKILL);
    close(read_from_dynamic);
    pthread_join(dynamic, nullptr);


}
/**
 * @brief Function to handle updates.
 *
 * @param sig The signal received.
 */
void update_prepare(int sig)
{

    if (sig == SIGUSR1) {
        syslog(LOG_INFO, "GOT UPDATE SIG\n");
        set_update_ti = 1;
        /* Reset signal handling to default behavior */
        signal(SIGUSR1, SIG_DFL);
    }
}
/**
 * @brief Thread for time-related system calls.
 *
 * @param a File descriptor of the pipe that is connected to the main.
 * @returns At exit.
 */
void* dynamic_thrd(void* a)
{
    thread_Data dynamic = *(thread_Data*)a;
    uint8_t dummy = 1;
    uint8_t counter = 0;
    char line[256];
    bool rest_php_pipe = true;
    FILE* fp;
    char getDynamic_php[] = "php getDynamic10.php\0";
    DaemonStatus::instance()->startThread(DaemonThreads::DYNAMIC);
    while (running)
    {
        if (!(counter % 2))
        {
            int pid = fork();
            if (pid == 0)// fork off to set wifi signal
            {
                int fd;
                for (fd = static_cast<int>(sysconf(_SC_OPEN_MAX)); fd > 0; fd--) {
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
        // calling php getDynamic10.php, to get the data for get_dynamic function
        // getDynamic10.php outputs 10 times 
        // this is done to reduce the system load that php call produces
        // in case of error feeds the pipe to main with dummy byte which generates error in get_dynamic function to
        // 
        if (running)
            fp = popen(getDynamic_php, "r");
        if (fp == nullptr)
        {
            syslog(LOG_ERR, "dynamic error can not make php call \n");
            continue;
        }
        int call_check = 0;
        while (fgets(line, 256, fp) != NULL && running)
        {
            write(dynamic.pipe_out, &line, 256);
            call_check++;
            g_sec_counter++;
            // call for update temp correction class
            delta_correction.update_Time(g_sec_counter);
            DaemonStatus::instance()->dataWasReceive(DaemonThreads::DYNAMIC);
        }
        if (call_check < 10)
        {
            syslog(LOG_ERR, "dynamic error call_check\n");
        }
        if (running)
            pclose(fp);
        counter++; 
    }
    close(dynamic.pipe_out);
    DaemonStatus::instance()->stopThread(DaemonThreads::DYNAMIC);
    return nullptr;
}
/**
 * @brief Thread that handles UART data from NRF and sends it to main, as well as sends data from main to NRF via UART.
 *
 * @param a A pointer to a structure containing:
 *          - file descriptor of the pipe end that is connected to main
 *          - file descriptor of the pipe end that is connected from main
 * @returns At exit.
 */
void* nrf_uart_thrd(void* a)
{
    return nullptr;
}

/**
* @brief Thread that handles uart data from ti and sends it to main,sends data from main to ti via uart and handles rf commuincation with external sensors
* @param a A pointer to a structure containing :
* -file descriptor of the pipe end that is connected to main
* -file descriptor of the pipe end that is connected from main
* @returns At exit.
*/
void* ti_uart_thrd(void* a)
{
    thread_Data ti = *(thread_Data*)a;
    pollfd fds[2];
    int num_open_fds = 2;
    fds[0].fd = open(TI_SERRIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    fds[0].events = POLLIN;
    termios tty;
    if (tcgetattr(fds[0].fd, &tty) != 0)
    {
        syslog(LOG_ERR, "Error: TI uart fd setup error\n");
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
        syslog(LOG_ERR, "Error: Save TI tty settings, also checking for error\n");
        return nullptr;
    }
    signal(SIGUSR1, update_prepare);
    //setup pipe
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
    //recives cmds from main and sends them to TI
    //handles rf communication with external sensors
    //feeds TI watchdog so the device is stays turned on
    //sets TI watchdog to LONG timeout when the daemon is stopped 
    //@note TI watchdog is fed every time TI recieves uart packet, Get_addr sets the watchdog to SHORT timeout
    //montiors for SIGUSR1 signal from system to set TI in UPDATE mode (not used)
    DaemonStatus::instance()->startThread(DaemonThreads::TI);
    while (running)
    {
        int ready;
        ready = poll(fds, 2, 1000);
        if (set_update_ti)
        {
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = set_update;
            tx_packet.ACK = ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;
            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
            set_update_ti = 0;
            syslog(LOG_INFO, "sending set_update");
            if (ready < 1)
            {
                ready = poll(fds, 2, 1000);
            }
        }
        if (ready != -1) //
        {
            if (ready > 0)
            {

                uint8_t buf[256];
                if (fds[0].revents & POLLIN) // uart
                {
                    DaemonStatus::instance()->dataWasReceive(DaemonThreads::TI);
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
                    if (PayloadLen > 0) {
                        memcpy(&rx_packet_ti.DataArray[0], &rx_data.RxDataArray[DATA_Offset], PayloadLen);
                    }
                    rx_packet_ti.CRC = (uint16_t)rx_data.RxDataArray[DATA_Offset + PayloadLen];
                    rx_packet_ti.CRC |= (uint16_t)(rx_data.RxDataArray[DATA_Offset + PayloadLen + 1] << 8);
                    uint16_t inc_crc_ti = crc16(rx_packet_ti.DataArray, rx_packet_ti.DataLen);
                    rx_packet_ti.PacketSrc = UART_Packet;
                    memset(&rx_data, 0, sizeof(rx_data));
                    if (inc_crc_ti == rx_packet_ti.CRC )
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
                            if (std::find(paired_list.begin(), paired_list.end(), *(uint32_t*)(rx_packet_ti.DataArray + 4)) != paired_list.end())
                            {
                                memcpy(rf_packet, &rx_packet_ti.DataArray[4], 4);
                                seq_num = rx_packet_ti.DataArray[8];
                                seq_num++;
                                memcpy(rf_packet + 8, &seq_num, 1);
                                dev_type = Main_dev;
                                memcpy(rf_packet + 9, &dev_type, 1);//Dev_type main
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
                                    buf[2] = AQS_DATA_SIZE +6;
                                    rf_len += AQS_THRSHLD_SIZE;
                                default:
                                    break;
                                }
                                memcpy(tx_packet.DataArray, rf_packet, rf_len); cpIndex = rf_len;
                                tx_packet.DataLen = cpIndex;

                                rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                                write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                                
                                if (memcmp(rx_packet_ti.DataArray, &brdcst_addr, 4))
                                    write(ti.pipe_out, buf, buf[2]+3);
                            }
                            else if(!memcmp(rx_packet_ti.DataArray, rf_packet + 4, 4))
                            {
                                memcpy(rf_packet, &rx_packet_ti.DataArray[4], 4);
                                seq_num = rx_packet_ti.DataArray[8];
                                seq_num++;
                                memcpy(rf_packet + 8, &seq_num, 1);
                                dev_type = NO_TYPE;//indicate to unpair 
                                memcpy(rf_packet + 9, &dev_type, 1);//Dev_type main
                                rf_len = 10;
                                memcpy(tx_packet.DataArray, rf_packet, rf_len); cpIndex = rf_len;
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
                        //case Check_Wiring:
                        case  SetRelay:
                        case  Send_packet:
                            buf[0] = rx_packet_ti.CMD;
                            buf[1] = rx_packet_ti.ACK;
                            buf[2] = rx_packet_ti.DataLen;
                            memcpy(buf+3, &rx_packet_ti.DataArray[0],rx_packet_ti.DataLen);
                            write(ti.pipe_out, buf, rx_packet_ti.DataLen+3);
                            break;
                        case GetInfo:
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = Get_addr;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            indx_rev = 0;
                            for (; rx_packet_ti.DataArray[indx_rev] != 0 && indx_rev<sizeof(rx_packet_ti.DataArray); indx_rev++)
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
                            memcpy(rf_packet+4, &rx_packet_ti.DataArray[0], 4);
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
                                syslog(LOG_ERR, "ERROR VERSION LENGTH\n");
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
                        case ShutDown:
                            //buf[0] = Shut_down;
                            //buf[1] = rx_packet_ti.ACK;
                            //buf[2] = rx_packet_ti.DataLen;
                            //write(ti.pipe_out, buf, 3);
                            break;
                        case feed_wtd:
                            break;
                        case set_update:
                            kill(getpid(),SIGTERM);
                            break;
                        default:
                            syslog(LOG_ERR, "ERROR_CMD %d\n", rx_packet_ti.CMD);
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
                       //thread to main pipe
                    
                       
                }
                else if (fds[0].revents & POLLERRVAL)
                {
                    syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"ti_pipe\": 0 }]}\n");
                    num_open_fds = 0;
                }
                if (fds[1].revents & POLLIN) // pipe. from main process
                {
                    ssize_t s = read(fds[1].fd, buf, sizeof(buf));//main to thread pipe
                    int indx_cm = 0;
                    //init
                    while (indx_cm<s)
                    {
                        switch (buf[indx_cm])
                        {
                        case Set_limits:
                            switch (buf[indx_cm+1])
                            {
                            case AQ_TH_PR:
                                memcpy(&throlds_aq, buf+ indx_cm + 2, AQS_THRSHLD_SIZE);
                                indx_cm+= (2 + AQS_THRSHLD_SIZE);
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
                        case  Set_paired:
                            paired_list.clear();
                            paired_list_rep_counter.clear();
                            for (int i = 0; i < buf[indx_cm+1]; i++)
                            {
                                paired_list.push_back(*(uint32_t*)(buf +indx_cm + 2 + 4 * i));
                                paired_list_rep_counter.push_back(0);
                            }
                            indx_cm += (2 + buf[indx_cm + 1] * 4);
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = StartPairing;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            //write(fds[0].fd, rf_tx_buff, rf_tx_buff_size); // 
                            break;
                        case Get_wiring://guaranteed to be the only wiring related cmd in a packet from main // not use? 
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = GetRelaySensor;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            tx_packet.DataLen = 0;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            indx_cm += 1;
                            break;
                        //case  Wiring_check://guaranteed to be the only wiring related cmd in a packet from main
                        //    tx_packet.PacketSrc = UART_Packet;
                        //    syslog(LOG_INFO, "DO: Wiring_check \n");
                        //    tx_packet.CMD = Check_Wiring;
                        //    tx_packet.ACK = ERROR_NO;
                        //    tx_packet.SID = 0x01;
                        //    tx_packet.DataLen = 0;
                        //    rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                        //    write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                        //    indx_cm += 1;
                        //    break;
                        case  Set_relays://guaranteed to be the only wiring related cmd in a packet from main
                            tx_packet.PacketSrc = UART_Packet;
                            tx_packet.CMD = SetRelay;
                            tx_packet.ACK = ERROR_NO;
                            tx_packet.SID = 0x01;
                            if ((static_cast<long>(indx_cm) + 1 + RELAY_OUT_CNT > s) || ((static_cast<unsigned long>(indx_cm) + 1 + RELAY_OUT_CNT) > sizeof(buf)))
                            {
                                syslog(LOG_ERR, "ERROR: SetRelay pipe_data read_size: %d, buf_size %d, index: %ld\n", s , sizeof(buf), (static_cast<unsigned long>(indx_cm) + 1 + RELAY_OUT_CNT));
                                indx_cm += (1 + RELAY_OUT_CNT);
                                break;
                            }
                            memcpy(tx_packet.DataArray, buf + indx_cm+ 1, RELAY_OUT_CNT);
                            tx_packet.DataLen = RELAY_OUT_CNT;
                            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            syslog(LOG_INFO, "DO: SetRelay [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d] \n", buf[indx_cm + 1], buf[indx_cm + 2], buf[indx_cm + 3], buf[indx_cm + 4], buf[indx_cm + 5], buf[indx_cm + 6], buf[indx_cm + 7], buf[indx_cm + 8], buf[indx_cm + 9], buf[indx_cm + 10]);
                            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            indx_cm += (1+ RELAY_OUT_CNT);
                            break;
                        case Shut_down: // cmd from backend
                            //tx_packet.PacketSrc = UART_Packet;
                            //tx_packet.CMD = ShutDown;
                            //tx_packet.ACK = ERROR_NO;
                            //tx_packet.SID = 0x01;
                            //tx_packet.DataLen = 0;
                            //rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                            //syslog(LOG_INFO, "DO: Cmd shutdown to TI\n");
                            //write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
                            //indx_cm += 1;
                            break;
                        default:
                            break;
                        }
                    }
                }
                else if ((fds[1].revents & POLLERRVAL))
                {
                    syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"ti_pipe\": 1 }]}\n");
                    //system needs to be restarted
                    fds[1].revents = 0;
                    num_open_fds=0;
                }

            }
            else
            {
                tx_packet.PacketSrc = UART_Packet;
                //syslog(LOG_INFO, "feed wdt");
                tx_packet.CMD = feed_wtd;
                tx_packet.ACK = ERROR_NO;
                tx_packet.SID = 0x01;
                tx_packet.DataLen = 0;
                rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
                write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
            }
        }
    }
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = set_wtd;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
    write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
    poll(fds, 1, 500);
    syslog(LOG_ALERT, "TI WTD set to Long");
    close(fds[1].fd);
    close(fds[0].fd);//system needs to be restarted
    DaemonStatus::instance()->stopThread(DaemonThreads::TI);
    return nullptr;
}
/**
*@brief emit_events key event to display
*/
void emit_event(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}
/**
*@brief Generates PAGE_UP key event
*/
void key_event()
{

    if (key_evt_file == 0)
    {
        
        key_evt_file = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
        struct uinput_setup usetup;
        /*
         * The ioctls below will enable the device that is about to be
         * created, to pass key events, in this case the space key.
         */

        if (ioctl(key_evt_file, UI_SET_EVBIT, EV_KEY)) {
            syslog(LOG_ERR, "UI_SET_EVBIT ioctl failed");
            return;
        }

        if (ioctl(key_evt_file, UI_SET_KEYBIT, KEY_PAGEUP)) {
            syslog(LOG_ERR, "UI_SET_KEYBIT ioctl failed");
            return;
        }
        memset(&usetup, 0, sizeof(usetup));
        usetup.id.bustype = BUS_VIRTUAL;
        usetup.id.vendor = 0x2333; /* sample vendor */
        usetup.id.product = 0x6666; /* sample product */
        usetup.id.version = 1; /* sample product */
        strcpy(usetup.name, "virtual device");
        if (ioctl(key_evt_file, UI_DEV_SETUP, &usetup)) {
            syslog(LOG_ERR, "UI_DEV_SETUP ioctl failed");
            quick_exit(0);
        }
        if (ioctl(key_evt_file, UI_DEV_CREATE)) {
            syslog(LOG_ERR, "UI_DEV_CREATE ioctl failed");
            quick_exit(0);
        }
         /*
         * On UI_DEV_CREATE the kernel will create the device node for this
         * device. We are inserting a pause here so that userspace has time
         * to detect, initialize the new device, and can start listening to
         * the event, otherwise it will not notice the event we are about
         * to send. This pause is only needed in our example code!
         */
        sleep(1);
    }
    if (fork() == 0)
    {
        system("export DISPLAY=:0");
        /* Key press, report the event, send key release, and report again */
        emit_event(key_evt_file, EV_KEY, KEY_PAGEUP, 1);
        emit_event(key_evt_file, EV_SYN, SYN_REPORT, 0);
        emit_event(key_evt_file, EV_KEY, KEY_PAGEUP, 0);
        emit_event(key_evt_file, EV_SYN, SYN_REPORT, 0);
        syslog(LOG_ERR, "KEY_PAGEUP");
        quick_exit(0);
    }
}

int main(int argc, char* argv[])
{
    int log_level = LOG_INFO;
    //makes this application a daemon 
//    daemonize();
    //sets cleanup as exit callback
    std::atexit(cleanup);
    //logging

    bool isArgValid = parseLogLevelOpt(argc, argv, log_level);
    openlog(argv[0], LOG_PID | LOG_CONS, LOG_DAEMON);
    if (isArgValid == false)
        syslog(LOG_ERR, "Log level argument invalid. Start with log level LOG_ERR\n");
    setlogmask(LOG_UPTO(log_level));
    
    syslog(LOG_INFO, "Started %s Version %s", argv[0], Daemon_Version);
    //change working directory to WEB_DIR
    chdir(WEB_DIR);

    if (!get_device_id())//init device id from linux
    {
        syslog(LOG_ERR, "Error: get_device_id \n");
    }
    device_t main_dev;
    main_dev.address = 0xffffffff; //this address is used in rf comms
    main_dev.paired = true;
    main_dev.type = Main_dev;
    Response_Time Rtv;
    Rtv.TP_internal_sesn_poll = 200;//2sec
    Rtv.TT_if_ack = 40;//10 min
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
    std::vector<uint8_t> relays_in_l, relays_in;
    //std::vector<uint8_t> wiring_state; //  not use. 
    bool shutdownFromDynamic = false; // for backend data parse
    // if shutdownFromDynamic true need to save cmd in case can't send it to ti on same iteration
    bool shutdown = false;


    uint8_t Thread_buff[256];
    SIO_Packet_t tx_packet,rx_packet;
    Serial_RxData_t packet_rx_settings;
    int Thread_send_size;
    int16_t nrf_Temp;
    int16_t nrf_Temp_far;
    uint8_t nrf_Hum;
    uint16_t nrf_aqs_CO2eq;
    uint16_t nrf_aqs_etoh;
    uint16_t nrf_aqs_TVOC;
    uint8_t  nrf_aqs_iaq;
    uint16_t  nrf_pressure;
    uint8_t brighness_mode=0;
    /// <summary>
    /// threading 
    /// create 2 threads with 2 pipes each 
    /// use poll for handling read write
    /// </summary>
    thread_Data nrf_vals, ti_vals,dynamic_vals;
    //bool wait_for_wiring_check = true;;
    int pipe_nrf_write_fd[2];
    pipe2(pipe_nrf_write_fd, O_NONBLOCK);
    int pipe_nrf_read_fd[2];
    pipe2(pipe_nrf_read_fd, O_NONBLOCK);
    int pipe_dynamic_fd[2];
    pipe2(pipe_dynamic_fd, O_NONBLOCK);
    dynamic_vals.pipe_out = pipe_dynamic_fd[1];
    nrf_vals.pipe_out = pipe_nrf_read_fd[1];
    nrf_vals.pipe_in = pipe_nrf_write_fd[0];
    read_from_nrf = pipe_nrf_read_fd[0];
    write_to_nrf = pipe_nrf_write_fd[1];
    read_from_dynamic = pipe_dynamic_fd[0];
    int pipe_ti_write_fd[2];
    pipe2(pipe_ti_write_fd, O_NONBLOCK);
    int pipe_ti_read_fd[2];
    pipe2(pipe_ti_read_fd, O_NONBLOCK);
    ti_vals.pipe_out = pipe_ti_read_fd[1];
    ti_vals.pipe_in = pipe_ti_write_fd[0];
    read_from_ti = pipe_ti_read_fd[0];
    write_to_ti = pipe_ti_write_fd[1];
    
    pollfd pipe_fds[3];
    pipe_fds[0].fd = read_from_nrf;
    pipe_fds[0].events = POLLIN;
    pipe_fds[1].fd = read_from_ti;
    pipe_fds[1].events = POLLIN;
    pipe_fds[2].fd = read_from_dynamic;
    pipe_fds[2].events = POLLIN;
    int num_open_fds = 3;
    //wait for web to be ready
    //Init the configs and necessary data from web
    while (running && !get_Config_list(time_configs))
    {
        sleep(1);
        syslog(LOG_INFO, "Waiting web for config list\n");
    };
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
    pthread_create(&ti, nullptr, ti_uart_thrd, (void*)&ti_vals);
    pthread_create(&dynamic, nullptr, dynamic_thrd, (void*)&dynamic_vals);
    pthread_create(&nrf, nullptr, nrf_uart_thrd, (void*)&nrf_vals);

    auto nrf_thread = new NRFThread((void*)&nrf_vals);
    nrf_thread->start();

    syslog(LOG_INFO, "WEB OK\n");
    if (!get_paired_list(device_list))//send to ti
    {
        syslog(LOG_ERR, "Error: get_paired \n");
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
        syslog(LOG_ERR, "Error: get_thresholds_list \n");
    }
    RGBm.red = 255;
    RGBm.blue = 255;
    RGBm.green = 255;
    RGBm.mode = LED_FADE;
    double delta;
    uint16_t RangeMilliMeter;
    uint16_t fan_speed;
    uint32_t Luminosity;
    bool pairing = false;
    bool last_pairing = false;
    bool wiring_check_f = false;
    int tmr_cntr = WIRING_CHECK_TIME;
    bool last_update_bool=true;
    bool last_update_bool_l=true;
    int error_dynamic_counter{};
    //set initial configs
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
    //AQ_TH_PR
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
    std::vector<std::string>::iterator pos;
    std::string const HWStr = VERSION_HW;
    std::string const VershStr = VERSION_HEAD;
    std::ofstream Wversion_fd;
    std::string line;
    std::vector<std::string> file_content;
    int haltCode;

    std::ifstream Rversion_fd;
    //hadles data from nrf and sets to web
    //handles data from ti and sets to web
    //handles data form web and sets to MCUs 
    //resrtarts the device or browser if necessary 
    //writes Hardware Version from nrf in the version file (web) 
    setBrightness(100);
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
                            // TODO?  more then one packet
                            break;
                        }
                    }
                    rx_packet.CMD = (SIO_Cmd_t)packet_rx_settings.RxDataArray[CMD_Offset];
                    rx_packet.ACK = packet_rx_settings.RxDataArray[ACK_Offset];
                    rx_packet.SID = packet_rx_settings.RxDataArray[SID_Offset];
                    uint8_t PayloadLen = packet_rx_settings.RxDataLen - PacketMinLength;
                    rx_packet.DataLen = PayloadLen;
                    if (PayloadLen > 0) {
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
                            syslog(LOG_ERR, "cmd:%d ack:%d\n", rx_packet.CMD, rx_packet.ACK);
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
                                //write 
                                Rversion_fd.open(VERSION_FILE);
                                if (Rversion_fd.is_open())
                                {
                                    while (getline(Rversion_fd, line))
                                    {
                                        file_content.push_back(line);
                                    }
                                    Rversion_fd.close();
                                }
                                else
                                {
                                    syslog(LOG_ERR, "COULD NOT OPEN VERSION FILE FOR READING\n");
                                }

                                 pos = std::find(file_content.begin(), file_content.end(), VershStr);
                                if (pos != file_content.end())
                                {
                                    pos+=2;
                                    if (pos->substr(0, HWStr.length()) == HWStr)
                                    {
                                        pos->erase(HWStr.length());
                                        pos->append(" "+NRF_HW);
                                        Wversion_fd.open(VERSION_FILE);
                                        if (Wversion_fd.is_open())
                                        {
                                            for (int i = 0; i < file_content.size(); i++)
                                            {
                                                Wversion_fd << file_content[i];
                                                if (i != file_content.size() - 1)
                                                {
                                                    Wversion_fd << '\n';
                                                }
                                            }
                                            Wversion_fd.close();
                                        }
                                        else
                                            syslog(LOG_ERR, "VERSION FILE ERROR WRITE\n");
                                    }
                                    else
                                        syslog(LOG_ERR, "VERSION FILE FORMAT ERROR HARDWARE \n");
                                }
                                else
                                {
                                    syslog(LOG_ERR, "VERSION FILE FORMAT ERROR HEADER\n");
                                }
                                    

                                break;
                            case   GetTOF:

                                memcpy(&RangeMilliMeter, rx_packet.DataArray, sizeof(RangeMilliMeter));
                                memcpy(&Luminosity, rx_packet.DataArray + sizeof(RangeMilliMeter), sizeof(Luminosity));
                                //syslog(LOG_INFO, "TOF works range: %d lum: %d\n", RangeMilliMeter, Luminosity);
                                //if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE)
                                //{
                                //    key_event();
                                //}
                                //if (brighness_mode == 1)
                                //{
                                //    if (!setBrightness(Luminosity))
                                //    {
                                //        syslog(LOG_INFO, "Error: setBrightness \n");
                                //    }
                                //}
                                saveBrTofTofile(RangeMilliMeter, Luminosity);
                                break;
                            case GetSensors:
                                cpIndex = 0;
                                memcpy(&nrf_Temp, rx_packet.DataArray + cpIndex, sizeof(nrf_Temp));

                                // correction temp. correction delta from 0.5 to 3.7 max.
                                delta = delta_correction.getDeltaTemp();
                                nrf_Temp_far = to_fahrenheit(nrf_Temp);
                                syslog(LOG_DEBUG, "DELTA: %f, temp raw fahr: %d \n", delta, nrf_Temp_far);
                                if (delta < 0.5 || delta > 3.7)
                                {
                                    syslog(LOG_ERR, "DELTA ERROR: %f\n", delta);
                                }
                                else
                                {
                                    nrf_Temp_far -= (delta * 10); // raw val in int * 10
                                    nrf_Temp = to_celsius(nrf_Temp_far); // back to cel for check alerts
                                }

                                memcpy(rx_packet.DataArray + cpIndex, &nrf_Temp_far, sizeof(nrf_Temp)); // far only for send. check alerts ib cel
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
                                if (!set_fan_speed_INFO(fan_speed))
                                {
                                    syslog(LOG_ERR, "Error: setFanSpeed\n");
                                }
                                //syslog(LOG_INFO, "GetSensors works range: %d lum: %d\n", RangeMilliMeter, Luminosity);

                                saveBrTofTofile(RangeMilliMeter, Luminosity);



                                //if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE)
                                //{
                                //    key_event();
                                //}

                                //if (brighness_mode == 1)
                                //{
                                //    if (!setBrightness(Luminosity))
                                //    {
                                //        syslog(LOG_INFO, "Error: setBrightness\n");
                                //    }
                                //}

                                //alert
                                {
                                    bool ret_cb=true;
                                    if (nrf_Temp > throlds_aq.temp_high * 10)
                                    {
                                        ret_cb=setAlert(0, Alert_temp_high, LVL_Emergency);
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
                                        syslog(LOG_ERR, "Error: setAlert \n");
                                    }
                                }
                                if (!setSensorData(main_dev, rx_packet.DataArray, rx_packet.DataLen))
                                {
                                    syslog(LOG_ERR, "Error: setSensorData \n");
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
                    syslog(LOG_EMERG, "{\"errno\": 12,\"data\":[{\"pipe\": 0 }]}\n");
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
                                //wait_for_wiring_check = true;
                                setAlert(0, Alert_wiring_not_connected, LVL_Emergency);
                                syslog(LOG_ERR, "ERROR_WIRING_NOT_CONNECTED\n");
                                syslog(LOG_ERR, "~%d ", Thread_buff[2]);
                                //buf[0] = Wiring_check;
                                //wait_resp = true;
                                //write(write_to_ti, buf, 1);//check wiring
                                break;
                            case ERROR_COULD_NOT_SET_RELAY:
                                syslog(LOG_ERR, "ERROR_COULD_NOT_SET_RELAY\n");
                                if (Thread_buff[2])
                                {
                                    syslog(LOG_ERR, "%d\n relay indx ", Thread_buff[2]);
                                    for (int k = 0; k < Thread_buff[2]; k++)
                                    {
                                        syslog(LOG_ERR, "%d ", Thread_buff[3+k]);
                                    }
                                    syslog(LOG_ERR, "\n");
                                }
                                
                                break;
                            }
                            break;
                            
                            syslog(LOG_INFO, "\n");
                        default:
                            break;
                        }
                        syslog(LOG_ERR, "cmd:%d ack:%d\n", Thread_buff[0], Thread_buff[1]);
                    }
                    else
                    {
                        device_t inc;
                        switch (Thread_buff[0])
                        {
                        case GetRelaySensor:
                            break;
                        //case Check_Wiring:
                        //    syslog(LOG_INFO, "Ret: Check_Wiring\n");
                        //    wait_for_wiring_check = false;
                        //    if (Thread_buff[2] == WIRING_IN_CNT)
                        //    {
                        //       
                        //        wait_for_wiring_check = false;
                        //        wiring_state.clear();
                        //        for (int i = 0; i < WIRING_IN_CNT; i++)
                        //        {
                        //            wiring_state.push_back(Thread_buff[3 + i]);
                        //        }
                        //        if (!setWiring(wiring_state))
                        //        {
                        //            syslog(LOG_INFO, "Error: setWiring \n");
                        //        }
                        //        syslog(LOG_INFO, "W:[%d, %d, %d, %d, %d, %d, %d, %d, %d, %d]\n", wiring_state[0], wiring_state[1], wiring_state[2], wiring_state[3], wiring_state[4], wiring_state[5], wiring_state[6], wiring_state[7], wiring_state[8], wiring_state[9]);
                        //        buf[0] = Set_relays;
                        //        for (int i = 0; i < RELAY_OUT_CNT; i++)
                        //        {
                        //            if (wiring_state[i] == WIRING_BROKEN && relays_in[i] == 1)
                        //            {
                        //                setAlert(0, Alert_wiring_not_connected, LVL_Emergency);
                        //                syslog(LOG_INFO, "wiring indx %d ", i);
                        //                buf[0] = NO_CMD;
                        //                relays_in_l = relays_in;
                        //                break;
                        //            }
                        //            buf[1 + i] = relays_in[i];
                        //        }
                        //        if(buf[0] != NO_CMD)
                        //        {
                        //            wait_resp = true;
                        //            write(write_to_ti, buf, 1 + RELAY_OUT_CNT);
                        //        }
                        //        
                        //        
                        //    }
                        //    break;
                        case  SetRelay:
                            syslog(LOG_INFO, "RET: SetRelay \n");
                            relays_in_l = relays_in;
                            break;
                        case  Send_packet:
                            break;
                        case Get_packets:
                            inc.address = *((uint32_t*)(Thread_buff + 5));
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
                                    
                                    //alert
                                    {
                                        bool ret_cb=true;
                                        if (aqthpr_dum_val.temp / 10.0 > throlds_aq.temp_high)
                                        {
                                            ret_cb=setAlert(inc.address, Alert_temp_high, LVL_Emergency);
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
                                            ret_cb = setAlert(inc.address, Alert_c02_high, LVL_Emergency);//here
                                        }
                                        else if (aqthpr_dum_val.Tvoc > throlds_aq.Tvoc_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_Tvoc_high, LVL_Emergency);//here
                                        }
                                        else if (aqthpr_dum_val.etoh > throlds_aq.etoh_high)
                                        {
                                            ret_cb = setAlert(inc.address, Alert_etoh_high, LVL_Emergency);
                                        }
                                        else if (aqthpr_dum_val.iaq / 10.0 > throlds_aq.iaq_high)
                                        {  
                                            ret_cb = setAlert(inc.address, Alert_iaq_high, LVL_Emergency);//here
                                        }
                                        else if (!ret_cb)
                                        {
                                            syslog(LOG_ERR, "Error: setAlert \n");
                                        }
                                    }
                                    aqthpr_dum_val.temp = to_fahrenheit(aqthpr_dum_val.temp);
                                    Thread_buff[13] = (aqthpr_dum_val.temp >> 8);
                                    Thread_buff[12] = static_cast<uint8_t>(aqthpr_dum_val.temp);
                                    if (!setSensorData(inc, &Thread_buff[9], AQS_DATA_SIZE))
                                    {
                                        syslog(LOG_ERR, "Error: setSensorData \n");
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
                                main_dev.address = *(uint32_t*)(Thread_buff + 3);

                            break;
                        case Shut_down:
                            //syslog(LOG_INFO, "DO: shutdown by backend cmd\n");
                            //haltCode = system("halt");
                            //if (haltCode != 0)
                            //{
                            //    openlog(argv[0], LOG_PID | LOG_CONS, LOG_DAEMON);
                            //    syslog(LOG_INFO, "Shutdown error. Halt return code: %d\n", haltCode);
                            //}
                            //while (1) 
                            //{ }
                            break;
                        default:
                            break;
                        }
                    }
                }
                else if (pipe_fds[1].revents & POLLERRVAL)
                {
                    syslog(LOG_EMERG, "{\"errno\": 12,\"data\":[{\"pipe\": 1 }]}\n");
                    close(pipe_fds[1].fd);
                    num_open_fds--;

                }
                //pipe event to get data from backend 
                if(pipe_fds[2].revents & POLLIN)
                {
                     ssize_t s = read(pipe_fds[2].fd, Thread_buff, sizeof(Thread_buff));
                    if (!get_dynamic(relays_in, pairing, check_paired_list, wiring_check_f, RGBm, brighness_mode, last_update_bool, Thread_buff, s, shutdownFromDynamic))
                    {
                        syslog(LOG_ERR, "Error: get_dynamic \n");
                        error_dynamic_counter++;
                        if (error_dynamic_counter == DEV_RESTART_TO)
                        {
                            syslog(LOG_EMERG, "Error: RESTARTING DEVICE NOW!!!\n");
                            //system("reboot&");
                        }
                        continue;
                    }
                    error_dynamic_counter = 0;
                    if (!last_update_bool)
                    {
                            syslog(LOG_INFO, "ERROR: Restarting browser and clearing chache\n");
                            system("rm - rf ~/.cache/QtExamples/quicknanobrowser/QtWebEngine/Profile/Cache/*");
                            system("systemctl restart xserver-nodm&");   
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
                            syslog(LOG_ERR, "Error: get_paired \n");
                        }
                        last_pairing = pairing;
                    }

                    
                    //if wiring_check needs to be performed delay Set_relays until after wiring_check returnes
                    //if ( wiring_check_f)
                    //{
                    //    
                    //    //
                    //    buf[cpIndex++] = Wiring_check;
                    //    
                    //}
                    // else if (relays_in != relays_in_l && !wait_for_wiring_check) // prev version
                    if (relays_in != relays_in_l) // cur version
                    {
                        uint8_t last_indx = cpIndex;
                        buf[cpIndex] = Set_relays;
                        for (int i = 0; i < RELAY_OUT_CNT; i++)
                        {
                            //if (wiring_state[i] == WIRING_BROKEN && relays_in[i] == 1)
                            //{
                            //    syslog(LOG_INFO, "ERROR_WIRING_NOT_CONNECTED\n");
                            //    syslog(LOG_INFO, "%d ", i);
                            //    buf[last_indx] = Wiring_check;//unset cmd
                            //    cpIndex = last_indx;
                            //    break;
                            //}
                            buf[++cpIndex] = relays_in[i];
                        }
                        cpIndex += 1;
                    }
                    // if one of prevs getDynamic call or current return shutdown. need to send it to ti. 
                    if (shutdown || shutdownFromDynamic)
                    {
                        //syslog(LOG_INFO, "SHUTDOWN START in main\n");
                        //
                        //shutdown = true; // just in case, if on current iteration couldn't send data. 
                        //buf[cpIndex++] = Shut_down;
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
                        syslog(LOG_INFO,"DO::RGB\n (%d,%d,%d) mode:%d\n", RGBm.red, RGBm.green, RGBm.blue, RGBm.mode);
                        write(write_to_nrf, Thread_buff, Thread_send_size);
                        RGBm_last.red = RGBm.red;
                        RGBm_last.green = RGBm.green;
                        RGBm_last.blue = RGBm.blue;
                        RGBm_last.mode = RGBm.mode;
                        double backlight_factor = (RGBm.red + RGBm.green + RGBm.blue) / 2.55 / 3.0;
                        if (backlight_factor >= MIN_BACKLIGHT_PERCENT_ON)
                        { // TODO any min val for ON?? 
                            syslog(LOG_DEBUG, "update_ON\n");
                            delta_correction.update_ON(g_sec_counter, backlight_factor / 100); // need 0.01 val
                        }
                        else
                        {
                            syslog(LOG_DEBUG, "update_OFF\n");
                            delta_correction.update_OFF(g_sec_counter);
                        }
                        //  update backlight
                        // if all 0 to off
                        // else to on
                           
                            
                    }
                }
                else if (pipe_fds[2].revents & POLLERRVAL)
                {
                    syslog(LOG_EMERG, "{\"errno\": 12,\"data\":[{\"pipe\": 2 }]}\n");
                    close(pipe_fds[2].fd);
                    num_open_fds--;

                }

            }
        }
        DaemonStatus::instance()->checkCurrentState();
    }
    syslog(LOG_EMERG, "END");
    exit(0);
}

