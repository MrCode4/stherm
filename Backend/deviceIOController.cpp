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
    if (nRfConnection)
        delete nRfConnection;

    if (tiConnection)
        delete tiConnection;
}

QVariantMap DeviceIOController::sendRequest(QString className, QString method, QVariantList data)
{
    qDebug() << "Request received: " << className << method << data;
    if (className == "hardware") {
        if (method == "setSettings") {
            if (data.size() < 0) {
                qWarning() << "data sent is empty";
            } else {
                if (data.size() != 6) {
                    qWarning() << "data sent is not consistent";
                }
                if (setBrightness(std::clamp(qRound(data.first().toDouble()), 0, 254)))
                    return {};
                else
                    return {{"error", true}};
            }
        }
    }

    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        //        connect(tiConnection, &UARTConnection::responseReceived, &loop, &QEventLoop::quit, Qt::SingleShotConnection);
        //        connect(tiConnection, &UARTConnection::connectionError, &loop, &QEventLoop::quit, Qt::SingleShotConnection);
        timer.start(10);

        QByteArray packet = mDataParser.preparePacket(STHERM::SIOCommand::GetInfo,
                                                      STHERM::PacketType::UARTPacket);
        if (tiConnection->sendRequest(packet)) {
            loop.exec();
        }
        qDebug() << "request finished";
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

    start();
}

void DeviceIOController::createSensor(QString name, QString id) {}

void DeviceIOController::run()
{
    QElapsedTimer timer;
    timer.start();
    while (!mStopReading) {
        qDebug() << "sending request for main data" << nRfConnection->isConnected();
        if (nRfConnection && nRfConnection->isConnected()) {
            //        uartConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
            QByteArray packet = mDataParser.preparePacket(STHERM::SIOCommand::GetSensors,
                                                          STHERM::PacketType::UARTPacket);
            nRfConnection->sendRequest(packet);
            //        uartConnection->sendRequest(STHERM::SIOCommand::GetTOF, STHERM::PacketType::UARTPacket);
        }

        //        if (tiConnection && tiConnection->isConnected())
        //            tiConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
        auto remainingTime = 3000 - timer.elapsed();
        if (remainingTime > 0)
            QThread::msleep(remainingTime);

        timer.restart();
    }
}

void DeviceIOController::createTIConnection()
{
    tiConnection = new UARTConnection(TI_SERRIAL_PORT, QSerialPort::Baud9600);
    if (tiConnection->startConnection()) {
        connect(tiConnection, &UARTConnection::sendData, this, [=](QByteArray data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "TI Response:   " << data;
            auto deserialized = mDataParser.deserializeTiData(data);
            qDebug() << Q_FUNC_INFO << __LINE__ << "TI Response:   " << deserialized;
        });
    }
}

void DeviceIOController::createNRF()
{
    nRfConnection = new UARTConnection(NRF_SERRIAL_PORT, QSerialPort::Baud9600);
    if (nRfConnection->startConnection()) {
        connect(nRfConnection, &UARTConnection::sendData, this, [=](QByteArray data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Response:   " << data;
            auto deserialized = mDataParser.deserializeNRFData(data);
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Response:   " << deserialized;

            // Set data to ui (Update ui varables).
            Q_EMIT dataReady(deserialized);
        });
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
