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
    case STHERM::SetColorRGB: {
        txPacket.DataLen = 5;
        bool on = data[4].toBool();
        txPacket.DataArray[0] = on ? std::clamp(data[0].toInt(), 0, 255) : 0;
        txPacket.DataArray[1] = on ? std::clamp(data[1].toInt(), 0, 255) : 0;
        txPacket.DataArray[2] = on ? std::clamp(data[2].toInt(), 0, 255) : 0;
        txPacket.DataArray[3] = 255;
        txPacket.DataArray[4] = data[3].toInt();
    } break;
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

STHERM::SIOPacket DataParser::prepareSIOPacket(STHERM::SIOCommand cmd, STHERM::PacketType packetType, QVariantList data)
{
    // Prepare packet to write
    STHERM::SIOPacket txPacket;
    txPacket.CMD = cmd;
    txPacket.PacketSrc = UtilityHelper::packetType(packetType);
    txPacket.ACK = STHERM::ERROR_NO;
    txPacket.SID = 0x01;
    txPacket.DataLen = 0;

    uint8_t dev_info[32];

    switch (cmd) {
    case STHERM::SetColorRGB: {
        txPacket.DataLen = 5;
        bool on = data[4].toBool();
        txPacket.DataArray[0] = on ? std::clamp(data[0].toInt(), 0, 255) : 0;
        txPacket.DataArray[1] = on ? std::clamp(data[1].toInt(), 0, 255) : 0;
        txPacket.DataArray[2] = on ? std::clamp(data[2].toInt(), 0, 255) : 0;
        txPacket.DataArray[3] = 255;
        txPacket.DataArray[4] = data[3].toInt();
    } break;
    case STHERM::InitMcus: {
        auto thresholds = data[0].value<STHERM::AQ_TH_PR_thld>();

        uint8_t cpIndex = 0;

        memcpy(txPacket.DataArray + cpIndex, &thresholds.temp_high, sizeof(thresholds.temp_high));
        cpIndex += sizeof(thresholds.temp_high);

        memcpy(txPacket.DataArray + cpIndex, &thresholds.temp_low, sizeof(thresholds.temp_low));
        cpIndex += sizeof(thresholds.temp_low);

        memcpy(txPacket.DataArray + cpIndex,
               &thresholds.humidity_high,
               sizeof(thresholds.humidity_high));
        cpIndex += sizeof(thresholds.humidity_high);

        memcpy(txPacket.DataArray + cpIndex,
               &thresholds.humidity_low,
               sizeof(thresholds.humidity_low));
        cpIndex += sizeof(thresholds.humidity_low);

        memcpy(txPacket.DataArray + cpIndex,
               &thresholds.pressure_high,
               sizeof(thresholds.pressure_high));
        cpIndex += sizeof(thresholds.pressure_high);

        memcpy(txPacket.DataArray + cpIndex, &thresholds.c02_high, sizeof(thresholds.c02_high));
        cpIndex += sizeof(thresholds.c02_high);

        memcpy(txPacket.DataArray + cpIndex, &thresholds.Tvoc_high, sizeof(thresholds.Tvoc_high));
        cpIndex += sizeof(thresholds.Tvoc_high);

        memcpy(txPacket.DataArray + cpIndex, &thresholds.etoh_high, sizeof(thresholds.etoh_high));
        cpIndex += sizeof(thresholds.etoh_high);
        txPacket.DataLen = cpIndex;
        //    TODO update CRC
    }
    default:
        break;
    }

    return txPacket;
}

STHERM::SIOPacket DataParser::deserializeData(const QByteArray &serializeData)
{
    TRACE_CHECK(false) << serializeData;

    STHERM::SerialRxData rxData;
    STHERM::SIOPacket  rxPacket;

    // Line = L
    // This function handles the received serial data, managing the escape
    // sequences and constructing the received packet
    // L 495-501 in ti / L 1069-1075 in NRF
    foreach (auto var, serializeData) {
        bool isFinished = UtilityHelper::SerialDataRx(var, &rxData);
        if (isFinished)
            break;
    }

    // Prepare SIOPacket packet with offsets
    rxPacket.CMD = (STHERM::SIOCommand)rxData.RxDataArray[CMD_Offset];
    rxPacket.ACK = rxData.RxDataArray[ACK_Offset];
    rxPacket.SID = rxData.RxDataArray[SID_Offset];
    uint8_t PayloadLen = rxData.RxDataLen - PacketMinLength;
    rxPacket.DataLen = PayloadLen;
    if (PayloadLen > 0) {
        // Copy response data to rxPacket
        // The DataArray contains the data to be processed.
        // position 0 to PayloadLen
        // ti: L 509 / NRF: L 1083
        memcpy(&rxPacket.DataArray[0], &rxData.RxDataArray[DATA_Offset], PayloadLen);
    }

    int crcOffset = DATA_Offset + PayloadLen;
    rxPacket.CRC = (uint16_t)rxData.RxDataArray[crcOffset];

    // 8 bits shift
    rxPacket.CRC |= (uint16_t)(rxData.RxDataArray[crcOffset + 1] << 8);

    rxPacket.PacketSrc = UART_Packet;
    memset(&rxData, 0, sizeof(rxData));


    return rxPacket;
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

