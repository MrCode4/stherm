#include "DataParser.h"


#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

// Offsets for various fields in serial packets
#define CMD_Offset      0
#define ACK_Offset      1
#define SID_Offset      2
#define DATA_Offset     3

// Minimum packet length
#define PacketMinLength 5

DataParser::DataParser(QObject *parent) :
    QObject(parent)
{
}

QByteArray DataParser::preparePacket(STHERM::SIOCommand cmd, STHERM::PacketType packetType)
{
    // Prepare packet to write
    STHERM::SIOPacket txPacket;
    txPacket.PacketSrc = UtilityHelper::packetType(packetType);
    txPacket.CMD = cmd;
    txPacket.ACK = STHERM::ERROR_NO;
    txPacket.SID = 0x01;
    txPacket.DataLen = 0;

    // An integer variable dev_buff_size is declared.
    int dev_buff_size;
    uint8_t dev_info[32];

    // It calls a function Set_SIO_TxPacket with the dev_info array and the
    // tx_packet structure. This function likely packages the data and the
    //  packet information into the dev_info array and returns the size
    //  of the data in dev_buff_size.
    dev_buff_size = UtilityHelper::setSIOTxPacket(dev_info, txPacket);

    QByteArray result = QByteArray::fromRawData(reinterpret_cast<char *>(dev_info),
                                                dev_buff_size);

    return result;
}

QVariantMap DataParser::deserializeData(const QByteArray &serializeData, const bool &isTi)
{
    if (isTi) {
        return deserializeTiData(serializeData);
    } else {
        return deserializeUARTData(serializeData);
    }
}

QVariantMap DataParser::deserializeUARTData(const QByteArray &serializeData)
{
    qDebug() << Q_FUNC_INFO << __LINE__ << serializeData;
    QJsonObject obj = QJsonDocument::fromJson(serializeData).object();
    QVariantMap mainData = obj.toVariantMap();

    return mainData;
}

QVariantMap DataParser::deserializeTiData(const QByteArray& serializeData)
{
    QJsonObject obj = QJsonDocument::fromJson(serializeData).object();
    QVariantMap mainData = obj.toVariantMap();
    STHERM::SerialRxData serialData;

    STHERM::SIOPacket tx_packet, rxPacketTi;

    foreach (auto var, serializeData) {
        bool isValid = UtilityHelper::SerialDataRx(static_cast<uint8_t>(var), &serialData);
        if (!isValid)
            break;
    }

    qDebug() << Q_FUNC_INFO << __LINE__ << serialData.RxDataArray;

    rxPacketTi.CMD = (STHERM::SIOCommand)serialData.RxDataArray[CMD_Offset];
    rxPacketTi.ACK = serialData.RxDataArray[ACK_Offset];
    rxPacketTi.SID = serialData.RxDataArray[SID_Offset];
    uint8_t PayloadLen = serialData.RxDataLen - PacketMinLength;
    rxPacketTi.DataLen = PayloadLen;
    if (PayloadLen > 0)
    {
        memcpy(&rxPacketTi.DataArray[0], &serialData.RxDataArray[DATA_Offset], PayloadLen);
    }
    rxPacketTi.CRC = (uint16_t)serialData.RxDataArray[DATA_Offset + PayloadLen];
    rxPacketTi.CRC |= (uint16_t)(serialData.RxDataArray[DATA_Offset + PayloadLen + 1] << 8);
    uint16_t inc_crc_ti = UtilityHelper::crc16(rxPacketTi.DataArray, rxPacketTi.DataLen);
    rxPacketTi.PacketSrc = UART_Packet;
    memset(&serialData, 0, sizeof(serialData));

    qDebug() << Q_FUNC_INFO << __LINE__ << rxPacketTi.CMD;

    return mainData;
}

