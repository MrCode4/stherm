#include "deviceIOController.h"

#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>

#include "UtilityHelper.h"

/* ************************************************************************************************
 * Device specifications
 * ************************************************************************************************/

#define NRF_GPIO_4 21
#define NRF_GPIO_5 22

#define NRF_SERRIAL_PORT "/dev/ttymxc1"
#define TI_SERRIAL_PORT "/dev/ttymxc3"

DeviceIOController::DeviceIOController(QObject *parent)
    : QObject{parent}
{
    this->moveToThread(&mThread);
    mThread.start();
}

QVariantMap DeviceIOController::sendRequest(QString className, QString method, QVariantList data)
{
    uartConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);

    // Uncomment to test
    //    uartConnection->sendRequest(STHERM::SIOCommand::GetSensors, STHERM::PacketType::UARTPacket);

    // Uncomment to test
    //    uartConnection->sendRequest(STHERM::SIOCommand::GetTOF, STHERM::PacketType::UARTPacket);

    // Uncomment to test
    tiConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);

    if (className == "hardware") {

        qDebug() << "Request received: " << className << method << data;

        if (method == "setSettings") {
            if (data.size() < 0) {
                qWarning() << "data sent is empty";
            } else {
                if (data.size() != 6) {
                    qWarning() << "data sent is not consistent";
                }
                setBrightness(std::clamp(qRound(data.first().toDouble()), 0, 254));
            }
        }
    }

    return {};
}

int DeviceIOController::getStartMode(int pinNumber)
{
    return UtilityHelper::getStartMode(pinNumber);
}

QString DeviceIOController::getCPUInfo()
{
    return UtilityHelper::getCPUInfo();
}

void DeviceIOController::setBrightness(int value)
{
    UtilityHelper::setBrightness(value);
}

void DeviceIOController::setTimeZone(int offset)
{
    UtilityHelper::setTimeZone(offset);
}

void DeviceIOController::createConnections()
{
    createNRF();
    createTIConnection();
}

void DeviceIOController::createSensor(QString name, QString id) {}

void DeviceIOController::createTIConnection()
{
    tiConnection = new UARTConnection(nullptr, true);

    tiConnection->initConnection(TI_SERRIAL_PORT, QSerialPort::Baud9600);
    if (tiConnection->connect() || true) { // CHECK: Remove '|| True'
        connect(tiConnection, &UARTConnection::sendData, this, [=](const QVariantMap &data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "TI Responce:   " << data;
        });

        tiConnection->start();
    }
}

void DeviceIOController::createNRF()
{
    uartConnection = new UARTConnection(nullptr, false);

    uartConnection->initConnection(NRF_SERRIAL_PORT, QSerialPort::Baud9600);
    if (uartConnection->connect() || true) { // CHECK: Remove '|| True'
        connect(uartConnection, &UARTConnection::sendData, this, [=](const QVariantMap &data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Responce:   " << data;
            // Set data to ui (Update ui varables).
            Q_EMIT dataReady(data);
        });

        uartConnection->start();
    }

    bool isSuccess = UtilityHelper::configurePins(NRF_GPIO_4);
    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Pin configuration failed: pin =" << NRF_GPIO_4;
    }

    isSuccess = UtilityHelper::configurePins(NRF_GPIO_5);
    if (!isSuccess) {
        qDebug() << Q_FUNC_INFO << __LINE__ << "Pin configuration failed: pin =" << NRF_GPIO_5;
    }
}
