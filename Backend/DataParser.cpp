#include "DataParser.h"


#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#define NRF_GPIO_4		21
#define NRF_GPIO_5		22


#define NRF_SERRIAL_PORT "/dev/ttymxc1"

DataParser::DataParser(QObject *parent) :
    QObject(parent)
{

    createNRF();
}

void DataParser::createNRF()
{
    uartConnection = new UARTConnection();

    uartConnection->initConnection(NRF_SERRIAL_PORT, QSerialPort::Baud9600);
    if (uartConnection->connect() || true) { // CHECK: Remove '|| True'
        connect(uartConnection, &UARTConnection::sendData, this, [=](const QByteArray& data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Responce:   " << data;
            QVariantMap mainData = deserializeMainData(data);
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Responce (Converted data):   " << mainData;
            emit dataReay(mainData);

        });
    }

    bool isSuccess =  UtilityHelper::configurePins(NRF_GPIO_4);
    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Pin configuration failed: pin =" <<NRF_GPIO_4;
    }

    isSuccess =  UtilityHelper::configurePins(NRF_GPIO_5);
    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Pin configuration failed: pin =" <<NRF_GPIO_5;
    }
}

void DataParser::sendRequest(STHERM::SIOCommand cmd, STHERM::PacketType packetType)
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

    uartConnection->writeData(reinterpret_cast<char*>(dev_info), dev_buff_size);
}

QVariantMap DataParser::deserializeMainData(const QByteArray& serializeData)
{
    QJsonObject obj = QJsonDocument::fromJson(serializeData).object();
    QVariantMap mainData = obj.toVariantMap();

    return mainData;
}

