//
// Created by Mortie on 11/2/2023.
//

#include "TIThread.h"
#include "DaemonStatus.h"
#include "Daemon_helper.h"

int TIThread::set_update_ti = 0;

TIThread::TIThread(void *a, QObject *parent) : QThread(parent) {
    ti = *(thread_Data*)a;
}

TIThread::~TIThread() {

}

void TIThread::run() {
    pollfd fds[2];
    int num_open_fds = 2;
    fds[0].fd = open(TI_SERRIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    fds[0].events = POLLIN;
    termios tty;
    if (tcgetattr(fds[0].fd, &tty) != 0)
    {
        syslog(LOG_ERR, "Error: TI uart fd setup error\n");
        exit(1);
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
        exit(1);
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

    m_initialized = true;

    while (m_initialized)
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
    exit(1);
}

void update_prepare(int sig) {

    if (sig == SIGUSR1) {
        syslog(LOG_INFO, "GOT UPDATE SIG\n");
        TIThread::set_update_ti = 1;
        /* Reset signal handling to default behavior */
        signal(SIGUSR1, SIG_DFL);
    }
}
