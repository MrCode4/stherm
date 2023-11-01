/**
 * @file serial_drv.c
 * @brief Implementation of serial data receive and transmit functions.
 *
 * This file contains functions for receiving and transmitting serial data,
 * including handling of special characters and calculating the CRC.
 */
#include "serial_drv.h"
#include "crc.h"
 /**
  * @brief Processes a received byte of serial data.
  *
  * This function handles the received serial data, managing the escape
  * sequences and constructing the received packet.
  *
  * @param RxData The received data byte.
  * @param[out] RxDataCfg The Serial_RxData_t configuration struct containing
  *                       the current state of the received packet.
  * @return True if the packet is successfully received and has a valid length,
  *         false otherwise.
  */
bool SerialDataRx(uint8_t RxData, Serial_RxData_t* RxDataCfg) {
    switch (RxData) {
        case phyStart: {
            RxDataCfg->RxDataLen = 0;
            RxDataCfg->RxActive = true;
            RxDataCfg->RxPacketDone = false;
            RxDataCfg->RxCtrlEsc = false;
        } break;
        case phyStop: {
            RxDataCfg->RxActive = false;
            RxDataCfg->RxPacketDone = true;
            RxDataCfg->RxCtrlEsc = false;
        } break;
        case phyCtrlEsc: {
            RxDataCfg->RxCtrlEsc = true;
        } break;
        default: {
            if (RxDataCfg->RxActive == true) {
                if (RxDataCfg->RxCtrlEsc == true) {
                    RxDataCfg->RxCtrlEsc = false;
                    RxDataCfg->RxDataArray[RxDataCfg->RxDataLen] = phyXorByte ^ RxData;
                } else {
                    RxDataCfg->RxDataArray[RxDataCfg->RxDataLen] = RxData;
                }
                RxDataCfg->RxDataLen++;
            }
        } break;
    }
    if (RxDataCfg->RxPacketDone == true) {
        RxDataCfg->RxPacketDone = false;
        if (RxDataCfg->RxDataLen >= PacketMinLength) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}
/**
 * @brief Prepares a serial packet for transmission.
 *
 * This function takes a packet of data and processes it for transmission,
 * including calculating the CRC and adding escape sequences for special
 * characters.
 *
 * @param[out] TxDataBuf The buffer containing the processed packet to be
 *                        transmitted.
 * @param TxPacket The SIO_Packet_t struct containing the original packet data.
 * @return The length of the processed packet in bytes.
 */
uint16_t Set_SIO_TxPacket(uint8_t* TxDataBuf, SIO_Packet_t TxPacket) {
    uint8_t tmpTxBuffer[256];
    uint16_t index = 0;
    uint16_t packetLen = 0;
    
    tmpTxBuffer[0] = TxPacket.CMD;
    tmpTxBuffer[1] = TxPacket.ACK;
    tmpTxBuffer[2] = TxPacket.SID;
    index += 3;
    memcpy(&tmpTxBuffer[index], &TxPacket.DataArray[0], TxPacket.DataLen);
    index += TxPacket.DataLen;
    TxPacket.CRC = crc16(&TxPacket.DataArray[0], TxPacket.DataLen);
    memcpy(&tmpTxBuffer[index], &TxPacket.CRC, 2);
    index += 2;

    TxDataBuf[packetLen++] = phyStart;

    int a=0;
    for(;a<index;a++)
    {
        if ( (tmpTxBuffer[a] == phyStart) || (tmpTxBuffer[a] == phyStop) || (tmpTxBuffer[a] == phyCtrlEsc) )
        {
            TxDataBuf[packetLen++] = phyCtrlEsc;
            TxDataBuf[packetLen++] = tmpTxBuffer[a] ^ phyXorByte;
        }
        else {
            TxDataBuf[packetLen++] = tmpTxBuffer[a];
        }
    }
    TxDataBuf[packetLen++] = phyStop;

    return packetLen;
}

