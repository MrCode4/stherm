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

#define TOF_IRQ_RANGE 1000    // mm

DeviceIOController::DeviceIOController(QObject *parent)
    : QThread{parent}
{
    // Prepare main device
    mMainDevice.address = 0xffffffff; // this address is used in rf comms
    mMainDevice.paired = true;
    mMainDevice.type = STHERM::Main_dev;

    nrfWaitForResponse = false;
    brighness_mode = 1;
}

DeviceIOController::~DeviceIOController()
{
    wtd_timer.stop();
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
            TRACE << "sending setBacklight request with data:" << data;
            if (data.size() == 5) {
                packet = mDataParser.preparePacket(STHERM::SIOCommand::SetColorRGB,
                                                   STHERM::PacketType::UARTPacket,
                                                   data);
                isRequestSent = nRfConnection->sendRequest(packet);
                TRACE << (QString("send setBacklight request: %0").arg(isRequestSent));

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

    connect(&wtd_timer, &QTimer::timeout, this, [this]() {
        static bool inProgress = false;
        if (inProgress) {
            return;
        }
        inProgress = true;
        TRACE << "start wtd" << (tiConnection && tiConnection->isConnected());

        if (tiConnection && tiConnection->isConnected()) {
            QByteArray packet = mDataParser.preparePacket(STHERM::SIOCommand::feed_wtd,
                                                          STHERM::PacketType::UARTPacket);
            bool success = false;
            QEventLoop loop;
            connect(tiConnection,
                    &UARTConnection::sendData,
                    this,
                    [&loop, &success](QByteArray data) {
                        success = true;
                        TRACE << "Ti heartbeat message success";
                        loop.quit();
                    });
            connect(tiConnection,
                    &UARTConnection::connectionError,
                    this,
                    [&loop, &success](QString error) {
                        success = false;
                        TRACE << "Ti heartbeat message failure";
                        loop.quit();
                    });

            bool rsp = tiConnection->sendRequest(packet);
            if (rsp == false) {
                TRACE << "Ti heartbeat message send failed";
            } else {
                TRACE << "Ti heartbeat message sent";
            }

            QTimer timer;
            timer.setSingleShot(true);
            connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            timer.start(3000);
            loop.exec();
            TRACE << "Ti heartbeat message finished";
        }

        TRACE << "start GetSensors" << (nRfConnection && nRfConnection->isConnected());
        if (nRfConnection && nRfConnection->isConnected()) {
            QByteArray packet = mDataParser.preparePacket(STHERM::SIOCommand::GetSensors,
                                                          STHERM::PacketType::UARTPacket);
            auto sent = nRfConnection->sendRequest(packet);

            TRACE << "nrf GetSensors message sent" << sent;
        }
        inProgress = false;
    });

    wtd_timer.setInterval(3000);
    wtd_timer.setSingleShot(false);
    wtd_timer.start();

    //    start();
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
    QString gpioValuePath = "/sys/class/gpio/gpio%1/value";

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
            TRACE << "NRF Response:   " << data;
            auto rxPacket = mDataParser.deserializeNRFData(data);
            LOG_DEBUG(QString("NRF Response - CMD: %0").arg(rxPacket.CMD));
            processNRFResponse(rxPacket);


            // Set data to ui (Update ui variables).
//            Q_EMIT responseReady(rxPacket.CMD, );
        });

        nrfConfiguration();
    }

    // TODO why are we trying to open a GPIO as a UART, we are testing! if not working should use a linux lowlevel code
    if (gpio4Connection->startConnection()) {
        connect(gpio4Connection, &UARTConnection::sendData, this, [=](QByteArray data) {
            TRACE << QString("gpio4Connection Response:   %0").arg(data);

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
            TRACE << QString("gpio4Connection Response:   %0").arg(data);

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

void DeviceIOController::processNRFResponse(STHERM::SIOPacket rxPacket)
{

//    QVariantMap resultMap;
    uint16_t inc_crc_nrf = UtilityHelper::crc16(rxPacket.DataArray, rxPacket.DataLen);
    int indx_rev = 0;

    uint16_t RangeMilliMeter;
    uint16_t fanSpeed;
    uint32_t Luminosity;

    // check data integridy
    if (inc_crc_nrf == rxPacket.CRC) {
        if (rxPacket.ACK == STHERM::ERROR_NO) {

            switch (rxPacket.CMD)
            {
            case STHERM::GetInfo: {
                indx_rev = 0;
                for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++)
                {
                    //                    NRF_HW.push_back(static_cast<char>(rx_packet.DataArray[indx_rev]));
                }
                ++indx_rev;
                for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++)
                {
                    //                    NRF_SW.push_back(static_cast<char>(rx_packet.DataArray[indx_rev]));
                }
                //                syslog(LOG_INFO, "NRF:: HW:%s SW:%s\n", NRF_HW.c_str(), NRF_SW.c_str());
            } break;

            case STHERM::GetTOF: {
                memcpy(&RangeMilliMeter, rxPacket.DataArray, sizeof(RangeMilliMeter));
                memcpy(&Luminosity, rxPacket.DataArray + sizeof(RangeMilliMeter), sizeof(Luminosity));

//                resultMap.insert("RangeMilliMeter", RangeMilliMeter);
//                resultMap.insert("Luminosity", Luminosity);

                LOG_DEBUG(QString("RangeMilliMeter (%0), Luminosity (%1)").arg(RangeMilliMeter).arg(Luminosity));
                if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE) {
                    // key_event('n');
                    LOG_DEBUG(QString("RangeMilliMeter (%0):  60 < RangeMilliMeter <= 1000 mm").arg(RangeMilliMeter));
                }

                if (brighness_mode == 1) {
                    if (!UtilityHelper::setBrightness(Luminosity)) {
                        LOG_DEBUG(QString("Error: setBrightness (Brightness: %0)").arg(Luminosity));
                    }
                }

            } break;

            case STHERM::GetSensors: {
                int cpIndex = 0;

                STHERM::AQ_TH_PR_vals mainDataValues;
                memcpy(&mainDataValues.temp, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.temp));
                cpIndex += sizeof(mainDataValues.temp);
                LOG_DEBUG(QString("mainDataValues.temp: %0").arg(mainDataValues.temp));

                memcpy(&mainDataValues.humidity, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.humidity));
                cpIndex += sizeof(mainDataValues.humidity);
                LOG_DEBUG(QString("mainDataValues.humidity: %0").arg(mainDataValues.humidity));

                memcpy(&mainDataValues.c02, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.c02));
                cpIndex += sizeof(mainDataValues.c02);
                LOG_DEBUG(QString("mainDataValues.c02: %0").arg(mainDataValues.c02));

                memcpy(&mainDataValues.etoh, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.etoh));
                cpIndex += sizeof(mainDataValues.etoh);
                LOG_DEBUG(QString("mainDataValues.etoh: %0").arg(mainDataValues.etoh));

                memcpy(&mainDataValues.Tvoc, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.Tvoc));
                cpIndex += sizeof(mainDataValues.Tvoc);
                LOG_DEBUG(QString("mainDataValues.Tvoc: %0").arg(mainDataValues.Tvoc));

                memcpy(&mainDataValues.iaq, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.iaq));
                cpIndex += sizeof(mainDataValues.iaq);
                LOG_DEBUG(QString("mainDataValues.iaq: %0").arg(mainDataValues.iaq));

                memcpy(&mainDataValues.pressure, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.pressure));
                cpIndex += sizeof(mainDataValues.pressure);
                LOG_DEBUG(QString("mainDataValues.pressure: %0").arg(mainDataValues.pressure));

                memcpy(&RangeMilliMeter, rxPacket.DataArray + cpIndex, sizeof(RangeMilliMeter));
                cpIndex += sizeof(RangeMilliMeter);
                LOG_DEBUG(QString("RangeMilliMeter: %0").arg(RangeMilliMeter));

                memcpy(&Luminosity, rxPacket.DataArray + cpIndex, sizeof(Luminosity));
                cpIndex += sizeof(Luminosity);
                LOG_DEBUG(QString("Luminosity: %0").arg(Luminosity));

                memcpy(&fanSpeed, rxPacket.DataArray + cpIndex, sizeof(fanSpeed));
                cpIndex += sizeof(fanSpeed);
                LOG_DEBUG(QString("fan_speed: %0").arg(fanSpeed));

                // todo
                //                if (!set_fan_speed_INFO(fan_speed)) {
                //                    LOG_DEBUG(QString("Error: setFanSpeed: (fan speed: %0)").arg(fan_speed));
                //                }
                // todo
                //                if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE) {
                //                    key_event('n');
                //                }

                if (brighness_mode == 1) {
                    if (!setBrightness(Luminosity)) {
                        LOG_DEBUG(QString("Error: setBrightness (Brightness: %0)").arg(Luminosity));
                    }
                }

                checkMainDataAlert(mainDataValues);

                // todo
                //                if (!setSensorData(main_dev, rx_packet.DataArray, rx_packet.DataLen)) {
                //                    LOG_DEBUG(QString("Error: setSensorData"));
                //                }
            } break;

            default:
                break;
            }

        } else {
            // Log error
            LOG_DEBUG(QString("cmd:%0 ACK:%1").arg(rxPacket.CMD).arg(rxPacket.ACK));
        }
    } else {
        // Log error
        LOG_DEBUG(QString("ACK and CRC are distinct. ACK:%0 CRC:%1").arg(rxPacket.ACK).arg(rxPacket.CRC));
    }
}

void DeviceIOController::checkMainDataAlert(const STHERM::AQ_TH_PR_vals &values)
{
    if (values.temp / 10.0 > AQ_TH_PR_thld.temp_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_temp_high);

    }  else if (values.temp / 10.0 < AQ_TH_PR_thld.temp_low) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_temp_low);

    } else if (values.humidity > AQ_TH_PR_thld.humidity_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_humidity_high);

    } else if (values.humidity < AQ_TH_PR_thld.humidity_low) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_humidity_low);

    } else if (values.pressure > AQ_TH_PR_thld.pressure_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_pressure_high);

    } else if (values.c02 > AQ_TH_PR_thld.c02_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_c02_high);

    } else if (values.Tvoc > AQ_TH_PR_thld.Tvoc_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_Tvoc_high);

    } else if (values.etoh > AQ_TH_PR_thld.etoh_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_etoh_high);

    } else if (values.iaq / 10.0 > AQ_TH_PR_thld.iaq_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_iaq_high);
    }
}


STHERM::ResponseTime DeviceIOController::getTimeConfig()
{
    // Time configuration
    STHERM::ResponseTime rtv;
    rtv.TP_internal_sesn_poll = 200; // 2sec
    rtv.TT_if_ack  = 40;              // 10 min
    rtv.TT_if_nack = 25;

    return rtv;
}

QList<STHERM::SensorTimeConfig> DeviceIOController::getSensorTimeConfig()
{
    QList<STHERM::SensorTimeConfig> configs;

    // Read from config file

    return configs;
}

QList<STHERM::SensorConfigThresholds> DeviceIOController::getSensorThresholds()
{
    QList<STHERM::SensorConfigThresholds> sensorsThresholds;


    return sensorsThresholds;
}
