#include "deviceIOController.h"

#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>

#include "LogHelper.h"

/* ************************************************************************************************
 * Device specifications
 * ************************************************************************************************/

#define NRF_GPIO_4 21
#define NRF_GPIO_5 22

#define NRF_SERIAL_PORT "/dev/ttymxc1"
#define TI_SERIAL_PORT  "/dev/ttymxc3"

DeviceIOController::DeviceIOController(QObject *parent)
    : QThread{parent}
{
    // Prepare main device
    mMainDevice.address = 0xffffffff; // this address is used in rf comms
    mMainDevice.paired = true;
    mMainDevice.type = STHERM::Main_dev;

    nrfWaitForResponse = false;

    // Time configuration
    STHERM::ResponseTime Rtv;
    Rtv.TP_internal_sesn_poll = 200; // 2sec
    Rtv.TT_if_ack = 40;              // 10 min
    Rtv.TT_if_nack = 25;



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

void DeviceIOController::nrfConfiguration()
{
    mSensorPacketBA = DataParser::preparePacket(STHERM::GetSensors);
    mTOFPacketBA    = DataParser::preparePacket(STHERM::GetTOF);
    // set initial configs

    // Send GetInfo request
    QByteArray packetBA = DataParser::preparePacket(STHERM::GetInfo);
    nRfConnection->sendRequest(packetBA);


    // Send InitMcus request
    STHERM::SIOPacket txPacket;
    txPacket.PacketSrc = UART_Packet;
    txPacket.CMD = STHERM::InitMcus;
    txPacket.ACK = STHERM::ERROR_NO;
    txPacket.SID = 0x01;

    STHERM::AQ_TH_PR_thld throldsAQ;
    uint8_t cpIndex = 0;

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.temp_high, sizeof(throldsAQ.temp_high));
    cpIndex += sizeof(throldsAQ.temp_high);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.temp_low, sizeof(throldsAQ.temp_low));
    cpIndex += sizeof(throldsAQ.temp_low);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.humidity_high, sizeof(throldsAQ.humidity_high));
    cpIndex += sizeof(throldsAQ.humidity_high);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.humidity_low, sizeof(throldsAQ.humidity_low));
    cpIndex += sizeof(throldsAQ.humidity_low);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.pressure_high, sizeof(throldsAQ.pressure_high));
    cpIndex += sizeof(throldsAQ.pressure_high);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.c02_high, sizeof(throldsAQ.c02_high));
    cpIndex += sizeof(throldsAQ.c02_high);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.Tvoc_high, sizeof(throldsAQ.Tvoc_high));
    cpIndex += sizeof(throldsAQ.Tvoc_high);

    memcpy(txPacket.DataArray + cpIndex, &throldsAQ.etoh_high, sizeof(throldsAQ.etoh_high));
    cpIndex += sizeof(throldsAQ.etoh_high);
    txPacket.DataLen = cpIndex;

    uint8_t ThreadBuff[256];
    int ThreadSendSize = UtilityHelper::setSIOTxPacket(ThreadBuff, txPacket);


    packetBA = QByteArray::fromRawData(reinterpret_cast<char *>(ThreadBuff),
                                       ThreadSendSize);
    nRfConnection->sendRequest(packetBA);
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

        QByteArray packet;

        bool isRequestSent = false;

        // todo: Add a function to check data
        if (className == "hardware" && method == "setBacklight") {

            if (data.size() == 5) {
                packet = mDataParser.preparePacket(STHERM::SIOCommand::GetInfo,
                                                   STHERM::PacketType::UARTPacket,
                                                   data);
                isRequestSent = nRfConnection->sendRequest(packet);
                LOG_DEBUG(QString("send setBacklight request: %0").arg(isRequestSent));

            } else {
                qWarning() << "data is empty or not consistent";
            }

        } else {
            qWarning() << "No class/method processor defined";
//            packet = mDataParser.preparePacket(STHERM::SIOCommand::GetInfo,
//                                               STHERM::PacketType::UARTPacket);
//            isRequestSent = tiConnection->sendRequest(packet);
        }

        if (isRequestSent) {
            loop.exec();
        }

        qDebug() << "request timeout";
    }

    return {};
}

int DeviceIOController::getStartMode(int pinNumber)
{
    return UtilityHelper::getGpioValue(pinNumber);
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

        if (tiConnection && tiConnection->isConnected())
        {
            //            tiConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
//            QByteArray rsp = tiConnection->sendCommand(STHERM::SIOCommand::feed_wtd);
            bool rsp = tiConnection->sendRequest(STHERM::SIOCommand::feed_wtd, STHERM::PacketType::UARTPacket);
            if (rsp == false) {
                qDebug() << "Ti heartbeat message failed";
            }
        }
        auto remainingTime = 3000 - timer.elapsed();
        if (remainingTime > 0)
            QThread::msleep(remainingTime);

        timer.restart();
    }
}

void DeviceIOController::createTIConnection()
{
    tiConnection = new UARTConnection(TI_SERIAL_PORT, QSerialPort::Baud9600);
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
    QString gpioValuePath = "/sys/class/gpio/gpio%1/value\0";

    bool isSuccess = UtilityHelper::configurePins(NRF_GPIO_4);
    if (isSuccess) {
        gpio4Connection = new UARTConnection(gpioValuePath.arg(NRF_GPIO_4), QSerialPort::Baud9600);
    } else {
        LOG_DEBUG(QString("Pin configuration failed: pin = %0").arg(NRF_GPIO_4));
        return;
    }

    isSuccess = UtilityHelper::configurePins(NRF_GPIO_5);
    if (isSuccess) {
        gpio5Connection = new UARTConnection(gpioValuePath.arg(NRF_GPIO_5), QSerialPort::Baud9600);

    } else {
        LOG_DEBUG(QString("Pin configuration failed: pin = %0").arg(NRF_GPIO_5));
        return;
    }

    nRfConnection = new UARTConnection(NRF_SERIAL_PORT, QSerialPort::Baud9600);
    if (nRfConnection->startConnection()) {
        connect(nRfConnection, &UARTConnection::sendData, this, [=](QByteArray data) {
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Response:   " << data;
            auto deserialized = mDataParser.deserializeNRFData(data);
            qDebug() << Q_FUNC_INFO << __LINE__ << "UART Response:   " << deserialized;

            // Set data to ui (Update ui varables).
            Q_EMIT dataReady(deserialized);
        });

        nrfConfiguration();
    }

// TODO why are we trying to open a GPIO as a UART
    if (gpio4Connection->startConnection()) {
        connect(gpio4Connection, &UARTConnection::sendData, this, [=](QByteArray data) {
            LOG_DEBUG(QString("gpio4Connection Response:   %0").arg(data));

            // Check (Read data after seek or ...)
            gpio4Connection->seek(SEEK_SET);

            if (data.length() == 2 && data.at(2) == '0') {
                nRfConnection->sendRequest(mSensorPacketBA);
            }
        });
    }

// TODO why are we trying to open a GPIO as a UART
    if (gpio5Connection->startConnection()) {
        connect(gpio5Connection, &UARTConnection::sendData, this, [=](QByteArray data) {
            LOG_DEBUG(QString("gpio4Connection Response:   %0").arg(data));

            // Check (Read data after seek or ...)
            gpio5Connection->seek(SEEK_SET);

            if (data.length() == 2 && data.at(2) == '0') {
                nRfConnection->sendRequest(mTOFPacketBA);
            }
        });
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
