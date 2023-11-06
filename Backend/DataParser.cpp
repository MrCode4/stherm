#include "DataParser.h"


#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#include "UtilityHelper.h"

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

QVariantMap DataParser::deserializeMainData(const QByteArray& serializeData)
{
    QJsonObject obj = QJsonDocument::fromJson(serializeData).object();
    QVariantMap mainData = obj.toVariantMap();

    return mainData;
}

