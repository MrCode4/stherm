#include "deviceIOController.h"

#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>

/* ************************************************************************************************
 * Device specifications
 * ************************************************************************************************/

#define NRF_GPIO_4 21
#define NRF_GPIO_5 22

#define NRF_SERRIAL_PORT "/dev/ttymxc1"
#define TI_SERRIAL_PORT "/dev/ttymxc3"

DeviceIOController::DeviceIOController(QObject *parent)
    : QThread{parent}
{
    // Prepare main device
    mMainDevice.address = 0xffffffff; // this address is used in rf comms
    mMainDevice.paired = true;
    mMainDevice.type = STHERM::Main_dev;
}

DeviceIOController::~DeviceIOController()
{
    mStopReading = true;
    terminate();
    wait();
    if (uartConnection)
        delete uartConnection;

    if (tiConnection)
        delete tiConnection;
}

bool DeviceIOController::sendRequest(QString className, QString method, QVariantList data)
{
    if (className == "hardware") {

        qDebug() << "Request received: " << className << method << data;

        if (method == "setSettings") {
            if (data.size() < 0) {
                qWarning() << "data sent is empty";
            } else {
                if (data.size() != 6) {
                    qWarning() << "data sent is not consistent";
                }
                return setBrightness(std::clamp(qRound(data.first().toDouble()), 0, 254));
            }
        }
    } else {
        tiConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);

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

bool DeviceIOController::setBrightness(int value)
{
    return UtilityHelper::setBrightness(value);
}

void DeviceIOController::setTimeZone(int offset)
{
    UtilityHelper::setTimeZone(offset);
}

bool DeviceIOController::setVacation(const int &minTemp, const int &maxTemp, const int &minHumidity, const int &maxHumidity)
{
    return true;
}

void DeviceIOController::createConnections()
{
    createNRF();
    createTIConnection();

    mStopReading = false;

    this->start();
}

void DeviceIOController::createSensor(QString name, QString id) {}

void DeviceIOController::run()
{
    QElapsedTimer timer;
    timer.start();
    while (!mStopReading) {

    if (uartConnection && uartConnection->isConnected()) {
        uartConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
//        uartConnection->sendRequest(STHERM::SIOCommand::GetSensors, STHERM::PacketType::UARTPacket);
//        uartConnection->sendRequest(STHERM::SIOCommand::GetTOF, STHERM::PacketType::UARTPacket);
    }

    if (tiConnection && tiConnection->isConnected())
        tiConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
    }

    auto remainingTime = 10000 - timer.elapsed();
    if (false && remainingTime > 0)
    QThread::msleep(remainingTime);
}

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

void DeviceIOController::setStopReading(bool stopReading)
{
    mStopReading = stopReading;
}

void DeviceIOController::updateTiDevices()
{
    // Temperature sensor

    // humidity sensor

    // Tof sensor

    // Ambient sensor

    // CO2 sensor

    qDebug() << Q_FUNC_INFO << __LINE__ << "Device count:   " << mDevices.count();
}
