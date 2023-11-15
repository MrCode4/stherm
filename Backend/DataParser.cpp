#include "DataParser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include "LogHelper.h"

// Offsets for various fields in serial packets
#define CMD_Offset      0
#define ACK_Offset      1
#define SID_Offset      2
#define DATA_Offset     3

// Minimum packet length
#define PacketMinLength 5

const uint16_t pressure_high_value {1200};  ///< Pressure threshold high (up to 1200 hPa)
const uint16_t c02_high_value      {2000};  ///< CO2 threshold high (400 to 5000 ppm)
const uint8_t  Tvoc_high_value     {50};    ///< TVOC threshold high (0.1 to 10+ mg/m^3)
const uint8_t  etoh_high_value     {70};    ///< ETOH threshold high (up to 20 ppm)
const uint8_t  iaq_high_value      {40};    ///< IAQ threshold high (1 to 5)
const int8_t   temp_high_value     {60};    ///< Temperature threshold high (up to +127 C)
const int8_t   temp_low_value      {-40};   ///< Temperature threshold low (as low as -128 C)
const uint8_t  humidity_high_value {80};    ///< Humidity threshold high (up to 100%)
const uint8_t  humidity_low_value  {10};    ///< Humidity threshold low (as low as 0%)

DataParser::DataParser(QObject *parent) :
    QObject(parent)
{
}

QByteArray DataParser::preparePacket(STHERM::SIOCommand cmd, STHERM::PacketType packetType, QVariantList data)
{
    // Prepare packet to write
    STHERM::SIOPacket txPacket;
    txPacket.CMD = cmd;
    txPacket.PacketSrc = UtilityHelper::packetType(packetType);
    txPacket.ACK = STHERM::ERROR_NO;
    txPacket.SID = 0x01;
    txPacket.DataLen = 0;

    // An integer variable dev_buff_size is declared.
    int dev_buff_size;
    uint8_t dev_info[32];

    switch (cmd) {
    case STHERM::SetColorRGB:
        txPacket.DataLen = 5;
        txPacket.DataArray[0] = std::clamp(data[0].toInt(), 0, 255);
        txPacket.DataArray[1] = std::clamp(data[1].toInt(), 0, 255);
        txPacket.DataArray[2] = std::clamp(data[2].toInt(), 0, 255);
        txPacket.DataArray[3] = 255;
        txPacket.DataArray[4] = data[3].toInt();
        TRACE << txPacket.DataArray[0] << txPacket.DataArray[1] << txPacket.DataArray[2]
              << txPacket.DataArray[3] << txPacket.DataArray[4];
        break;
    default:
        break;
    }

    // call Set_SIO_TxPacket with the output and input structures
    // return the size of the dev_info output packet
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
        return deserializeNRFData(serializeData);
    }
}

void DataParser::checkAlert(const STHERM::AQ_TH_PR_vals &values)
{
    if (values.temp / 10.0 > temp_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_temp_high);

    }  else if (values.temp / 10.0 < temp_low_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_temp_low);

    } else if (values.humidity > humidity_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_humidity_high);

    } else if (values.humidity < humidity_low_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_humidity_low);

    } else if (values.pressure > pressure_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_pressure_high);

    } else if (values.c02 > c02_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_c02_high);

    } else if (values.Tvoc > Tvoc_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_Tvoc_high);

    } else if (values.etoh > etoh_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_etoh_high);

    } else if (values.iaq / 10.0 > iaq_high_value) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_iaq_high);
    }
}

QVariantMap DataParser::deserializeNRFData(const QByteArray &serializeData)
{
    qDebug() << Q_FUNC_INFO << __LINE__ << serializeData;
    QJsonObject obj = QJsonDocument::fromJson(serializeData).object();

    qDebug() << Q_FUNC_INFO << __LINE__ << obj.toVariantMap();
    {
        QJsonObject obj;
        obj.insert("temp", 10);
        obj.insert("hum", 30.24);
        qDebug() << Q_FUNC_INFO << __LINE__ << obj.toVariantMap();
        return obj.toVariantMap();
    }

    return obj.toVariantMap();
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

    qDebug() << Q_FUNC_INFO << __LINE__ << inc_crc_ti << rxPacketTi.CRC;
    qDebug() << Q_FUNC_INFO << __LINE__ << rxPacketTi.CMD;

    if (inc_crc_ti == rxPacketTi.CRC)
    {
        switch (rxPacketTi.CMD)
        {
        case STHERM::Get_packets:
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = STHERM::Send_packet;
            tx_packet.ACK = STHERM::ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;
            break;
        case STHERM::GetRelaySensor:
        case STHERM::Check_Wiring:
        case STHERM::SetRelay:
        case STHERM::Send_packet:
            break;
        case STHERM::GetInfo:
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = STHERM::Get_addr;
            tx_packet.ACK = STHERM::ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;
            break;
        case STHERM::Get_addr:
            break;
        case STHERM::GET_DEV_ID:
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = STHERM::GET_DEV_ID;
            tx_packet.ACK = STHERM::ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;
            break;
        case STHERM::feed_wtd:

            break;
        default:
            break;
        }
    }

    return mainData;
}

STHERM::AQ_TH_PR_vals DataParser::AQTHPRFromBytes(const QByteArray &bytes)
{
    STHERM::AQ_TH_PR_vals aqthpr_dum_val;

    aqthpr_dum_val.Tvoc = bytes[9];
    aqthpr_dum_val.etoh = bytes[10];
    aqthpr_dum_val.iaq = bytes[11];
    aqthpr_dum_val.temp = static_cast<uint16_t>((bytes[13] << 8) | bytes[12]);
    aqthpr_dum_val.humidity = bytes[14];
    aqthpr_dum_val.c02 = static_cast<uint16_t>((bytes[16] << 8) | bytes[15]);
    aqthpr_dum_val.pressure = static_cast<uint16_t>((bytes[18] << 8) | bytes[17]);

    return aqthpr_dum_val;
}

