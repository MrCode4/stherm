/**
 * @file Luminosity_report.h
 * @brief This file contains the main function responsible for acquiring the Luminosity value from a TOF sensor.
 *
 * The program sets up UART communication with an nRF device and sends a command to read TOF (Time of Flight) data
 * from the connected sensor. The program retrieves Luminosity and Range (in millimeters) values, and then outputs
 * the Luminosity value.
 */
#include "Peripheral.h"

 /**
  * @brief Main function for acquiring the Luminosity value from a TOF sensor.
  *
  * This function initializes UART communication with an nRF device, sends a command to read TOF data from
  * the sensor, and retrieves the Luminosity and Range (in millimeters) values. The Luminosity value is then printed.
  *
  * @return int Returns 0 if the process is completed, non-zero error code if unsuccessful.
  */
int main()
{
    pollfd fds[1];
    Serial_RxData_t packet_rx_settings;
    SIO_Packet_t  rx_packet;
    fds[0].fd = open(NRF_SERRIAL_PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    fds[0].events = POLLIN;
    termios tty;
    if (tcgetattr(fds[0].fd, &tty) != 0) {
        printf("-1\n");
        return 0;
    }
    set_tty(&tty);
    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    // Save tty settings, also checking for error
    if (tcsetattr(fds[0].fd, TCSANOW, &tty) != 0) {
        printf("-2\n");
        return 0;
    }
    //setup pipe fd
    uint8_t dfu_tx_buff[32];
    SIO_Packet_t tx_packet;
    int dfu_tx_buff_size;
    tx_packet.PacketSrc = UART_Packet;
    tx_packet.CMD = GetTOF;
    tx_packet.ACK = ERROR_NO;
    tx_packet.SID = 0x01;
    tx_packet.DataLen = 0;
    dfu_tx_buff_size = Set_SIO_TxPacket(dfu_tx_buff, tx_packet);
    ssize_t s;
    int ready;
    uint8_t buf[256];
    uint32_t Luminosity;
    uint16_t RangeMilliMeter;
    fds[0].revents = 0;
    write(fds[0].fd, dfu_tx_buff, dfu_tx_buff_size);
    ready = 0;
    bool done_flag = false;

        ready = poll(fds, 1, 10000);
        if (ready > 0)
        {

            if (fds[0].revents & POLLIN)
            {
                s = read(fds[0].fd, buf, sizeof(buf));
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
                if (PayloadLen > 0) {
                    memcpy(&rx_packet.DataArray[0], &packet_rx_settings.RxDataArray[DATA_Offset], PayloadLen);
                }
                rx_packet.CRC = (uint16_t)packet_rx_settings.RxDataArray[DATA_Offset + PayloadLen];
                rx_packet.CRC |= (uint16_t)(packet_rx_settings.RxDataArray[DATA_Offset + PayloadLen + 1] << 8);
                uint16_t inc_crc_nrf = crc16(rx_packet.DataArray, rx_packet.DataLen);
                rx_packet.PacketSrc = UART_Packet;
                memset(&packet_rx_settings, 0, sizeof(packet_rx_settings));
                if (inc_crc_nrf == rx_packet.CRC)
                {
                    if (rx_packet.ACK != ERROR_NO)
                    {
                        printf("-3\n");
                        return 0;
                    }
                    else
                    {
                        switch (rx_packet.CMD)
                        {
                        case   GetTOF:
                            memcpy(&RangeMilliMeter, rx_packet.DataArray, sizeof(RangeMilliMeter));
                            memcpy(&Luminosity, rx_packet.DataArray + sizeof(RangeMilliMeter), sizeof(Luminosity));
                            printf("%d\n", Luminosity);

                            break;
                        }
                    }
                }


            }
            else if (fds[0].revents & POLLERRVAL)
            {
                fds[0].revents = 0;
                printf("-4\n");
                close(fds[0].fd);//system needs to be restarted
                return 0;
            }
        }
        else if (ready == 0)
        {
            printf("-5\n");
            return 0;
        }
    

    return 0;
}

