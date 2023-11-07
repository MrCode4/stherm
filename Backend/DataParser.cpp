#include "DataParser.h"


#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

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

QVariantMap DataParser::deserializeMainData(const QByteArray& serializeData)
{
    QJsonObject obj = QJsonDocument::fromJson(serializeData).object();
    QVariantMap mainData = obj.toVariantMap();

    return mainData;
}

