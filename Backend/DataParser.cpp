#include "DataParser.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
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
    if (uartConnection->connect()) {
        connect(uartConnection, &UARTConnection::sendData, this, [=](QString data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Responce:   " << data;
            QJsonObject obj = QJsonDocument::fromJson(data.toUtf8()).object();
            QVariantMap mainData = obj.toVariantMap();
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Responce (Converted data):   " << mainData;
            emit dataReay(mainData);

        });
    }

    bool isSuccess =  configurePins(NRF_GPIO_4);
    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Pin configuration failed: pin =" <<NRF_GPIO_4;
    }

    isSuccess =  configurePins(NRF_GPIO_5);
    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Pin configuration failed: pin =" <<NRF_GPIO_5;
    }
}

bool DataParser::configurePins(int gpio)
{
    // Update export file
    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open export file.";
        return false;
    }

    // Convert pinNumber to string
    QString pinString = QString::number(gpio);

    // Write the pin number to the export file
    QTextStream out(&exportFile);
    out << pinString;
    exportFile.close();


    // Update direction file
    QString directionFilePath = QString("/sys/class/gpio/gpio%0/direction").arg(gpio);
    QFile directionFile(directionFilePath);

    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open direction file for pin " << gpio;
        return false;
    }

    QTextStream outIn(&directionFile);
    outIn << "in";

    directionFile.close();

    // Update edge file
    QString edgeFilePath = QString("/sys/class/gpio/gpio%d/edge").arg(gpio);
    QFile edgeFile(directionFilePath);

    if (!edgeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Failed to open eadge file for pin " << gpio;
        return false;
    }

    QTextStream outInEdge(&edgeFile);
    outIn << "falling";

    edgeFile.close();

    return true;
}
