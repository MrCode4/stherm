//
// Created by Mortie on 11/1/2023.
//

#include "NRFThread.h"
#include "../DaemonStatus.h"
#include <QDebug>

#define SENS_POLL_TIME 5 //sec

NRFThread::NRFThread(void *a, QObject *parent)
    : QThread(parent)
{
    nrf = *(thread_Data*)a;
}

NRFThread::~NRFThread() {

}

void NRFThread::run() {
    pollfd fds[4];
    int num_open_fds = 4;
    time_t last_polled_sens = time(NULL);
    fds[0].fd = open(NRF_SERRIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    fds[0].events = POLLIN ;
    termios tty;
    if (tcgetattr(fds[0].fd, &tty) != 0) {
        syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        exit(1);
    }
    set_tty(&tty);
    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    // Save tty settings, also checking for error
    if (tcsetattr(fds[0].fd, TCSANOW, &tty) != 0) {
        syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        exit(1);
    }
    if (configure_pins(NRF_GPIO_4)) {
        syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        exit(1);
    }
    if (configure_pins(NRF_GPIO_5)) {;
        syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_uart\": 0 }]}\n");
        exit(1);
    }
    //setup pipe fd
    char str[36];
    sprintf(str, SW_VAL_PATH, NRF_GPIO_4);
    fds[1].fd = nrf.pipe_in;
    fds[1].events = POLLIN;
    fds[2].fd = open(str, O_RDONLY | O_NONBLOCK);
    fds[2].events = POLL_GPIO;
    sprintf(str, SW_VAL_PATH, NRF_GPIO_5);
    fds[3].fd = open(str, O_RDONLY| O_NONBLOCK);
    fds[3].events = POLL_GPIO;
    for (int p = 0; p < 4; p++)
    {
        fds[p].revents = 0;
    }
    uint8_t sens_tx_buff[32];
    uint8_t tof_tx_buff[32];
    uint8_t dev_info[32];
    SIO_Packet_t tx_packet;
    int sens_tx_buff_size;
    int tof_tx_buff_size;
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetInfo;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;

    int dev_buff_size;
    dev_buff_size = Set_SIO_TxPacket(dev_info, tx_packet);
    write(fds[0].fd, dev_info, dev_buff_size);
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetSensors;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    sens_tx_buff_size = Set_SIO_TxPacket(sens_tx_buff, tx_packet);
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetTOF;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    tof_tx_buff_size = Set_SIO_TxPacket(tof_tx_buff, tx_packet);

    ssize_t s;
    int ready;
    uint8_t buf[256];
    bool wait_for_response = true;
    //polls nrf for data every SENS_POLL_TIME seconds
//catches interups on NRF_GPIO_4 for sensonr data and NRF_GPIO_5 for tof or light data
//recived data is sent to main
//recives data from main and sends it to nrf
    DaemonStatus::instance()->startThread(DaemonThreads::NRF);

    m_initialized = true;

    while (m_initialized) {
        qDebug() << "NRF Thread running";

        ready = poll(fds, 4, SENS_POLL_TIME*1000+1500);
        if (ready != -1) //
        {
            if (ready > 0)
            {

                if (fds[0].revents & POLLIN) // from uart
                {
                    s = read(fds[0].fd, buf, sizeof(buf));
                    write(nrf.pipe_out, buf, s);
                    wait_for_response = false;
                    DaemonStatus::instance()->dataWasReceive(DaemonThreads::NRF);
                }
                else if (fds[0].revents & POLLERRVAL)
                {
                    fds[0].revents = 0;
                    syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 0 }]}\n");

                    num_open_fds = 0;
                    break;
                }
                if (fds[1].revents & POLLIN )
                {
                    if (!wait_for_response)
                    {
                        //data from main
                        s = read(fds[1].fd, buf, sizeof(buf));
                        write(fds[0].fd, buf, s);
                        wait_for_response = true;
                    }
                }
                else if (fds[1].revents & POLLERRVAL)
                {
                    syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 1 }]}\n");

                    fds[1].revents = 0;
                    num_open_fds=0;
                    break;
                }
                if (fds[2].revents & POLLPRI )
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
                    syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 2 }]}\n");

                    num_open_fds = 0;
                    break;
                }
                if (fds[3].revents & POLLPRI )
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
                    syslog(LOG_ERR, "{\"errno\": 12,\"data\":[{\"nrf_pipe\": 3 }]}\n");
                    num_open_fds = 0;
                    break;
                }

            }
            if((ready==0 || difftime(time(NULL), last_polled_sens) >= SENS_POLL_TIME) && !wait_for_response)
            {
                write(fds[0].fd, sens_tx_buff, sens_tx_buff_size);
                last_polled_sens = time(NULL);
                wait_for_response = true;
            }
        }
        else
        {
            syslog(LOG_ERR, "NRF thrd error -1\n");
            syslog(LOG_ERR, "%s", strerror(errno));
        }
    }
}
