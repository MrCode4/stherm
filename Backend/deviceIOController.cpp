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

#define WIRING_CHECK_TIME 600 // ms

DeviceIOController::DeviceIOController(QObject *parent)
    : QThread{parent}
{
    // Get device id
    getDeviceID();

    nRfConnection   = nullptr;
    tiConnection    = nullptr;
    gpio4Connection = nullptr;
    gpio5Connection = nullptr;

    // Prepare main device
    mMainDevice.address = 0xffffffff; // this address is used in rf comms
    mMainDevice.paired = STHERM::pair;
    mMainDevice.type = STHERM::Main_dev;

    nrfWaitForResponse = false;
    brighness_mode = 1;
}

DeviceIOController::~DeviceIOController()
{
    wiring_timer.stop();
    wtd_timer.stop();
    nRF_timer.stop();
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
    ///! initialize some structures

    STHERM::AQ_TH_PR_thld throldsAQ; // TODO getValues from settings?!

    mSensorPacketBA = DataParser::preparePacket(STHERM::GetSensors);
    mTOFPacketBA = DataParser::preparePacket(STHERM::GetTOF);

    ///! set initial configs

    ///! Send GetInfo request to initialize communication, TODO reply?

    QByteArray packetBA = DataParser::preparePacket(STHERM::GetInfo);
    nRfConnection->sendRequest(packetBA);

    ///! Send InitMcus request after GetInfo, TODO should we wait for reply?
    /// we can add this to a queue in constructor later and process the queue here
    STHERM::SIOPacket txPacket;
    txPacket.PacketSrc = UART_Packet;
    txPacket.CMD = STHERM::InitMcus;
    txPacket.ACK = STHERM::ERROR_NO;
    txPacket.SID = 0x01;

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
    //    TODO update CRC

    uint8_t ThreadBuff[256];
    int ThreadSendSize = UtilityHelper::setSIOTxPacket(ThreadBuff, txPacket);


    packetBA = QByteArray::fromRawData(reinterpret_cast<char *>(ThreadBuff),
                                       ThreadSendSize);
    nRfConnection->sendRequest(packetBA);
}

void DeviceIOController::tiConfiguration()
{
    // Send GetInfo request
    QByteArray packetBA = DataParser::preparePacket(STHERM::GetInfo);
    tiConnection->sendRequest(packetBA);

    ///! Send StartPairing request after GetInfo, TODO should we wait for reply?
    /// we can add this to a queue in constructor later and process the queue here
    /// this section is done different in daemon and passes multiple data to thread! which we do not need
    /// Set_limits and Set_time
    //    packetBA = DataParser::preparePacket(STHERM::StartPairing);
    //    tiConnection->sendRequest(packetBA);
}

inline bool sendRequestWithReply(UARTConnection *connection,
                                 STHERM::SIOCommand CMD,
                                 QVariantList data,
                                 int timeout_msec = 10)
{
    if (connection->property("busy").toBool()){
        TRACE << "Connection is busy";
        return false;
    }

    connection->setProperty("busy", true);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    loop.connect(&timer, &QTimer::timeout, &loop, [&loop]() {
        loop.setProperty("error", "timeout");
        loop.quit();
    });
    loop.connect(connection, &UARTConnection::sendData, &loop, &QEventLoop::quit);
    loop.connect(connection,
                 &UARTConnection::connectionError,
                 &loop,
                 [&loop](QString error_connection) {
                     loop.setProperty("error", error_connection);
                     loop.quit();
                 });

    timer.start(timeout_msec);

    auto packet = DataParser::preparePacket(CMD,
                                            STHERM::PacketType::UARTPacket,
                                            data);
    if (connection->sendRequest(packet)) {
        loop.exec();
    } else {
        loop.setProperty("error", "request not sent, aborting");
    }

    auto error = loop.property("error").toString();
    if (!error.isEmpty()) {
        qWarning() << "request failed:" << error << ", command: " << CMD << data;
    }

    connection->setProperty("busy", false);
    return error.isEmpty();
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
                if (setBrightness(std::clamp(qRound(data.first().toDouble()), 5, 255)))
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
            TRACE_CHECK(false) << "sending setBacklight request with data:" << data;
            if (nRfConnection && nRfConnection->isConnected() && data.size() == 5) {
                packet = mDataParser.preparePacket(STHERM::SIOCommand::SetColorRGB,
                                                   STHERM::PacketType::UARTPacket,
                                                   data);
                isRequestSent = nRfConnection->sendRequest(packet);
                TRACE_CHECK(false) << (QString("send setBacklight request: %0").arg(isRequestSent));

            } else {
                qWarning() << "data is empty or not consistent";
            }

        } else if (tiConnection && tiConnection->isConnected()) {
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

    connect(&wtd_timer, &QTimer::timeout, this, &DeviceIOController::wtdExec);

    wtd_timer.setSingleShot(false);
    wtd_timer.start(10000);

    connect(&wiring_timer, &QTimer::timeout, this, &DeviceIOController::wiringExec);

    wiring_timer.setSingleShot(false);
    wiring_timer.start(12000);

    ///! TODO we need o fix the scenario using GPIO later
    connect(&nRF_timer, &QTimer::timeout, this, &DeviceIOController::nRFExec);


    nRF_timer.setSingleShot(false);
    nRF_timer.start(1000);

    //    start();
}

void DeviceIOController::createSensor(QString name, QString id) {}

void DeviceIOController::run()
{
    QElapsedTimer timer;
    timer.start();

    static int wiringCheckTimer = WIRING_CHECK_TIME;

    while (!mStopReading) {

        if (nRfConnection) {
            qDebug() << "sending request for main data" << nRfConnection->isConnected();
            if (nRfConnection->isConnected()) {
                //        uartConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
                QByteArray packet = mDataParser.preparePacket(STHERM::SIOCommand::GetSensors,
                                                              STHERM::PacketType::UARTPacket);
                nRfConnection->sendRequest(packet);
                //        uartConnection->sendRequest(STHERM::SIOCommand::GetTOF, STHERM::PacketType::UARTPacket);
            }
        } else {
            qDebug() << Q_FUNC_INFO << __LINE__ ;
            LOG_DEBUG("nRfConnection cannot established");
        }

        if (tiConnection && tiConnection->isConnected())
        {
// TODO please dont duplex the timer like this as you're making independent tasks dependent on each other.  Please have 2 timers or checks instead
            wiringCheckTimer++;
            if (wiringCheckTimer > WIRING_CHECK_TIME) {
                bool rsp = tiConnection->sendRequest(STHERM::SIOCommand::Check_Wiring, STHERM::PacketType::UARTPacket);

                LOG_DEBUG(QString("Wiring check sent: ") + QString(rsp ? "true" : "false"));
                wiringCheckTimer = 0;

            } else {
                //            tiConnection->sendRequest(STHERM::SIOCommand::GetInfo, STHERM::PacketType::UARTPacket);
                //            QByteArray rsp = tiConnection->sendCommand(STHERM::SIOCommand::feed_wtd);
                bool rsp = tiConnection->sendRequest(STHERM::SIOCommand::feed_wtd, STHERM::PacketType::UARTPacket);
                if (rsp == false) {
                    qDebug() << "Ti heartbeat message failed";
                }
            }
        }

        qDebug() << Q_FUNC_INFO << __LINE__ ;
        auto remainingTime = 3000 - timer.elapsed();
        if (remainingTime > 0)
            QThread::msleep(remainingTime);

        timer.restart();
    }
}

void DeviceIOController::createTIConnection()
{
    tiConnection = new UARTConnection(TI_SERIAL_PORT, QSerialPort::Baud9600, true);
    if (tiConnection->startConnection()) {
        connect(tiConnection, &UARTConnection::sendData, this, [=](QByteArray data) {
            LOG_DEBUG(QString("Ti Response: %0").arg(data));

            auto rxPacket = mDataParser.deserializeData(data);

            LOG_DEBUG(QString("TI Response - CMD: %0").arg(rxPacket.CMD));
            processTIResponse(rxPacket);
        });

        tiConfiguration();
    }
}

void DeviceIOController::createNRF()
{

    bool isSuccess = UtilityHelper::configurePins(NRF_GPIO_4);
    if (!isSuccess) {
        LOG_DEBUG(QString("Pin configuration failed: pin = %0").arg(NRF_GPIO_4));
        return;
    }

    isSuccess = UtilityHelper::configurePins(NRF_GPIO_5);
    if (!isSuccess) {
        LOG_DEBUG(QString("Pin configuration failed: pin = %0").arg(NRF_GPIO_5));
        return;
    }

    GpioHandler *gpioHandler4 = new GpioHandler(NRF_GPIO_4, this);
    GpioHandler *gpioHandler5 = new GpioHandler(NRF_GPIO_5, this);

    nRfConnection = new UARTConnection(NRF_SERIAL_PORT, QSerialPort::Baud9600);
    if (nRfConnection->startConnection()) {
        connect(nRfConnection, &UARTConnection::sendData, this, [=](QByteArray data) {
            TRACE_CHECK(false) << "NRF Response:   " << data;
            auto rxPacket = mDataParser.deserializeData(data);
            TRACE_CHECK(false) << (QString("NRF Response - CMD: %0").arg(rxPacket.CMD));
            processNRFResponse(rxPacket);


            // Set data to ui (Update ui variables).
//            Q_EMIT responseReady(rxPacket.CMD, );
        });

        nrfConfiguration();
    }

    if (!gpioHandler4->hasError() && false) {
        connect(gpioHandler4, &GpioHandler::readyRead, this, [=](QByteArray data) {
            TRACE << QString("gpio4Connection Response:   %0").arg(data) << data.length();

            if (data.length() == 2 && data.at(0) == '0') {
                TRACE << "request for gpio 4";
                //<< nRfConnection->sendRequest(mSensorPacketBA);
            }
        });
    }

    if (!gpioHandler5->hasError() && false) {
        connect(gpioHandler5, &GpioHandler::readyRead, this, [=](QByteArray data) {
            TRACE << QString("gpio5Connection Response:   %0").arg(data) << data.length();

            if (data.length() == 2 && data.at(0) == '0') {
                TRACE << "request for gpio 5" << nRfConnection->sendRequest(mTOFPacketBA);
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

bool DeviceIOController::setBacklight(QVariantList data)
{
    TRACE_CHECK(false) << "sending setBacklight request with data:" << data
                       << (nRfConnection && nRfConnection->isConnected());
    if (nRfConnection && nRfConnection->isConnected() && data.size() == 5) {
        auto result = sendRequestWithReply(nRfConnection,
                                           STHERM::SIOCommand::SetColorRGB,
                                           data,
                                           100);
        TRACE_CHECK(false) << "send setBacklight request" << result;
        return result;
    } else {
        qWarning() << "backlight not sent: data is empty or not consistent or nRF not connected";
    }

    return false;
}

bool DeviceIOController::setSettings(QVariantList data)
{
    if (data.size() <= 0) {
        qWarning() << "data sent is empty";
        return false;
    } else {
        if (data.size() != 6) {
            qWarning() << "data sent is not consistent";
            return false;
        }
        if (setBrightness(std::clamp(qRound(data.first().toDouble()), 5, 255)))
            return true;
        else
            return false;
    }
}

void DeviceIOController::wtdExec()
{
    TRACE << "start wtd" << (tiConnection && tiConnection->isConnected());

    // TODO: timer should be restart on every request and no reply waiting needed
    if (tiConnection && tiConnection->isConnected()) {
        auto rsp = sendRequestWithReply(tiConnection, STHERM::SIOCommand::feed_wtd,{}, 10000);

        if (rsp == false) {
            TRACE << "Ti heartbeat message send failed";
        } else {
            TRACE << "Ti heartbeat message sent";
        }

        TRACE << "Ti heartbeat message finished" << rsp;
    }
}

void DeviceIOController::wiringExec()
{
    TRACE << "start Check_Wiring" << (tiConnection && tiConnection->isConnected());

    if (tiConnection && tiConnection->isConnected()) {
        auto rsp = sendRequestWithReply(tiConnection, STHERM::SIOCommand::Check_Wiring,{}, 10000);

        if (rsp == false) {
            TRACE << "Ti Check_Wiring message send failed";
        } else {
            TRACE << "Ti Check_Wiring message sent";
        }

        TRACE << "Ti Check_Wiring message finished" << rsp;
    }

}

void DeviceIOController::nRFExec()
{

    TRACE_CHECK(false) << "start NRF" << (nRfConnection && nRfConnection->isConnected());
    if (nRfConnection && nRfConnection->isConnected()) {

        TRACE_CHECK(false) << "start GetSensors";

        auto rsp = sendRequestWithReply(nRfConnection, STHERM::SIOCommand::GetSensors,{}, 10000);

        TRACE_CHECK(false) << "GetSensors message finished" << rsp;

        TRACE_CHECK(false) << "start GetTOF";
        auto packet = mDataParser.preparePacket(STHERM::SIOCommand::GetTOF,
                                                STHERM::PacketType::UARTPacket);
        auto sent = nRfConnection->sendRequest(packet);

        TRACE_CHECK(false) << "nrf GetTOF message sent" << sent;
    }
}

void DeviceIOController::processNRFResponse(STHERM::SIOPacket rxPacket)
{

//    QVariantMap resultMap;
    // checksum the response data
    uint16_t inc_crc_nrf = UtilityHelper::crc16(rxPacket.DataArray, rxPacket.DataLen);
    int indx_rev = 0;

    uint16_t RangeMilliMeter;
    uint16_t fanSpeed;
    uint32_t Luminosity;

    // check data integridy
    if (inc_crc_nrf == rxPacket.CRC) {
        if (rxPacket.ACK == STHERM::ERROR_NO) {

            switch (rxPacket.CMD) {
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
                // Read RangeMilliMeter and Luminosity
                // In OLD code: 1118-1132
                memcpy(&RangeMilliMeter, rxPacket.DataArray, sizeof(RangeMilliMeter));
                memcpy(&Luminosity, rxPacket.DataArray + sizeof(RangeMilliMeter), sizeof(Luminosity));

//                resultMap.insert("RangeMilliMeter", RangeMilliMeter);
//                resultMap.insert("Luminosity", Luminosity);

                TRACE_CHECK(false) <<(QString("RangeMilliMeter (%0), Luminosity (%1)").arg(RangeMilliMeter).arg(Luminosity));
                if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE) {
                    // key_event('n'); // send screen saver event
                    auto wake = new QEvent(QEvent::User);
                    QCoreApplication::instance()->sendEvent(QCoreApplication::instance(), wake);
                    TRACE_CHECK(false) <<(QString("RangeMilliMeter (%0):  60 < RangeMilliMeter <= 1000 mm").arg(RangeMilliMeter));
                }

                if (false && brighness_mode == 1) {
                    if (!setBrightness(Luminosity)) {
                        TRACE_CHECK(false) <<(QString("Error: setBrightness (Brightness: %0)").arg(Luminosity));
                    }
                }

            } break;

            case STHERM::GetSensors: {
                // Read data from DataArray
                // in ODL code: 1134-1227

                int cpIndex = 0;
                STHERM::AQ_TH_PR_vals mainDataValues;
                int16_t tempValue;
                memcpy(&tempValue, rxPacket.DataArray + cpIndex, sizeof(tempValue));
                cpIndex += sizeof(tempValue);
                mainDataValues.temp =  tempValue / 10.0; // in celsius
                TRACE_CHECK(false) << (QString("mainDataValues.temp: %0").arg(mainDataValues.temp));

                memcpy(&mainDataValues.humidity, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.humidity));
                cpIndex += sizeof(mainDataValues.humidity);
                TRACE_CHECK(false) <<(QString("mainDataValues.humidity: %0").arg(mainDataValues.humidity));

                memcpy(&mainDataValues.c02, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.c02));
                cpIndex += sizeof(mainDataValues.c02);
                TRACE_CHECK(false) <<(QString("mainDataValues.c02: %0").arg(mainDataValues.c02));

                memcpy(&mainDataValues.etoh, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.etoh));
                cpIndex += sizeof(mainDataValues.etoh);
                TRACE_CHECK(false) <<(QString("mainDataValues.etoh: %0").arg(mainDataValues.etoh));

                memcpy(&mainDataValues.Tvoc, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.Tvoc));
                cpIndex += sizeof(mainDataValues.Tvoc);
                TRACE_CHECK(false) <<(QString("mainDataValues.Tvoc: %0").arg(mainDataValues.Tvoc));

                uint8_t iaq;
                memcpy(&iaq, rxPacket.DataArray + cpIndex, sizeof(iaq));
                cpIndex += sizeof(iaq);
                mainDataValues.iaq = iaq / 10.0;
                TRACE_CHECK(false) <<(QString("mainDataValues.iaq: %0").arg(mainDataValues.iaq));

                memcpy(&mainDataValues.pressure, rxPacket.DataArray + cpIndex, sizeof(mainDataValues.pressure));
                cpIndex += sizeof(mainDataValues.pressure);
                TRACE_CHECK(false) <<(QString("mainDataValues.pressure: %0").arg(mainDataValues.pressure));

                memcpy(&RangeMilliMeter, rxPacket.DataArray + cpIndex, sizeof(RangeMilliMeter));
                cpIndex += sizeof(RangeMilliMeter);
                TRACE_CHECK(false) <<(QString("RangeMilliMeter: %0").arg(RangeMilliMeter));

                memcpy(&Luminosity, rxPacket.DataArray + cpIndex, sizeof(Luminosity));
                cpIndex += sizeof(Luminosity);
                TRACE_CHECK(false) <<(QString("Luminosity: %0").arg(Luminosity));

                memcpy(&fanSpeed, rxPacket.DataArray + cpIndex, sizeof(fanSpeed));
                TRACE_CHECK(false) <<(QString("fan_speed: %0").arg(fanSpeed));

                // Prepare data and send to ui
                QVariantMap mainDataMap;
                mainDataMap.insert("temperature",     mainDataValues.temp);
                mainDataMap.insert("humidity",        mainDataValues.humidity);
                mainDataMap.insert("co2",             mainDataValues.c02);
                mainDataMap.insert("etoh",            mainDataValues.etoh);
                mainDataMap.insert("Tvoc",            mainDataValues.Tvoc);
                mainDataMap.insert("iaq",             mainDataValues.iaq);
                mainDataMap.insert("pressure",        mainDataValues.pressure);
                mainDataMap.insert("RangeMilliMeter", RangeMilliMeter);
                mainDataMap.insert("brighness",       Luminosity);
                mainDataMap.insert("fanSpeed",        fanSpeed);

                emit mainDataReady(mainDataMap);

                // todo
                //                if (!set_fan_speed_INFO(fan_speed)) {
                //                    LOG_DEBUG(QString("Error: setFanSpeed: (fan speed: %0)").arg(fan_speed));
                //                }
                // todo
                //                if (RangeMilliMeter > 60 && RangeMilliMeter <= TOF_IRQ_RANGE) {
                //                    key_event('n');
                //                }

                if (false && brighness_mode == 1) {
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
            LOG_DEBUG(QString("ERROR: ACK shows an error in cmd:%0 (ACK:%1)").arg(rxPacket.CMD).arg(rxPacket.ACK));
        }
    } else {
        // Log error
        LOG_DEBUG(QString("Calculated CRC and CRC are distinct. Calculated CRC:%0 CRC:%1").arg(inc_crc_nrf).arg(rxPacket.CRC));
    }
}

void DeviceIOController::processTIResponse(STHERM::SIOPacket rxPacket)
{
    uint16_t inc_crc_ti = UtilityHelper::crc16(rxPacket.DataArray, rxPacket.DataLen);
    int indx_rev = 0;

    // Check to fill
    QList<quint32> paired_list;

    STHERM::SIOPacket tx_packet;

    // Use default values
    STHERM::ResponseTime Rtv = getTimeConfig();

    // Use default values
    STHERM::AQ_TH_PR_thld throlds_aq;
    uint8_t seq_num;
    uint8_t dev_type;
    uint8_t buf[256];
    uint8_t rf_len;
    uint8_t cpIndex = 0;
    uint32_t brdcst_addr = 0xffffffff;

    bool aquired_addr = false;

    if (inc_crc_ti == rxPacket.CRC) {
        switch (rxPacket.CMD) {
        case STHERM::Get_packets: {
            uint8_t rf_packet[128];
            if (!aquired_addr)
                break;
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = STHERM::Send_packet;
            tx_packet.ACK = STHERM::ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;
            if (std::find(paired_list.begin(), paired_list.end(), *(uint32_t *)(rxPacket.DataArray + 4)) != paired_list.end())
            {
                memcpy(rf_packet, &rxPacket.DataArray[4], 4);
                seq_num = rxPacket.DataArray[8];
                seq_num++;
                memcpy(rf_packet + 8, &seq_num, 1);
                dev_type = STHERM::Main_dev;
                memcpy(rf_packet + 9, &dev_type, 1); // Dev_type main
                switch (rxPacket.DataArray[9])
                {
                case STHERM::AQ_TH_PR: {
                    memcpy(rf_packet + 10, &Rtv, sizeof(Rtv));
                    rf_len = 10 + sizeof(Rtv);
                    memcpy(rf_packet + rf_len, &throlds_aq, AQS_THRSHLD_SIZE);

                    STHERM::AQ_TH_PR_vals aqthpr_dum_val;
                    aqthpr_dum_val.Tvoc = rxPacket.DataArray[10];
                    aqthpr_dum_val.etoh = rxPacket.DataArray[11];
                    aqthpr_dum_val.iaq = rxPacket.DataArray[12];
                    aqthpr_dum_val.temp = static_cast<uint16_t>((rxPacket.DataArray[14] << 8) | rxPacket.DataArray[13]);
                    aqthpr_dum_val.humidity = rxPacket.DataArray[15];
                    aqthpr_dum_val.c02 = static_cast<uint16_t>((rxPacket.DataArray[17] << 8) | rxPacket.DataArray[16]);
                    aqthpr_dum_val.pressure = static_cast<uint16_t>((rxPacket.DataArray[19] << 8) | rxPacket.DataArray[20]);

                    // CHECK ALERT
                    checkMainDataAlert(aqthpr_dum_val);
                    // Update model with aqthpr_dum_val
//                    setSensorData(inc, &Thread_buff[9], AQS_DATA_SIZE)

                } break;

                default:
                    break;
                }
                memcpy(tx_packet.DataArray, rf_packet, rf_len);
                cpIndex = rf_len;
                tx_packet.DataLen = cpIndex;

                uint8_t rf_tx_buff[256];
                uint16_t size = UtilityHelper::setSIOTxPacket(rf_tx_buff, tx_packet);

                QByteArray packet = QByteArray::fromRawData(reinterpret_cast<char *>(rf_tx_buff),
                                                            size);
                tiConnection->sendRequest(packet);

            } else if (!memcmp(rxPacket.DataArray, rf_packet + 4, 4)) {
                memcpy(rf_packet, &rxPacket.DataArray[4], 4);
                seq_num = rxPacket.DataArray[8];
                seq_num++;
                memcpy(rf_packet + 8, &seq_num, 1);
                dev_type = STHERM::NO_TYPE;          // indicate to unpair
                memcpy(rf_packet + 9, &dev_type, 1); // Dev_type main
                rf_len = 10;
                memcpy(tx_packet.DataArray, rf_packet, rf_len);
                cpIndex = rf_len;
                tx_packet.DataLen = cpIndex;

                uint8_t tx_buff[256];
                uint16_t size = UtilityHelper::setSIOTxPacket(tx_buff, tx_packet);

                QByteArray packet = QByteArray::fromRawData(reinterpret_cast<char *>(tx_buff),
                                                            size);
                tiConnection->sendRequest(packet);
            } else if (!memcmp(rxPacket.DataArray, &brdcst_addr, 4)) {
                STHERM::DeviceType inc;
                // Check
                inc.address = *((uint32_t *)(rxPacket.DataArray + 5));
                inc.paired = STHERM::DevicePairStatus::pending;
                inc.type = rxPacket.DataArray[9];

                addPendingSensor(inc);
            }
        } break;
        case STHERM::SetRelay: {
            if (rxPacket.ACK == STHERM::ERROR_NO) {
                LOG_DEBUG("***** Ti  - Start SetRelay *****");
                relays_in_l = relays_in;
                LOG_DEBUG("***** Ti  - SetRelay finished *****");
            } else {
                switch (rxPacket.ACK) {
                case STHERM::ERROR_WIRING_NOT_CONNECTED: {
                    //                    wait_for_wiring_check = true;
                    emit alert(STHERM::LVL_Emergency, STHERM::Alert_wiring_not_connected);
                    LOG_DEBUG("ERROR_WIRING_NOT_CONNECTED");
                    LOG_DEBUG("~" + QString::number(rxPacket.DataLen));
                    //                    wait_resp = true; prevent dynamic thread before response
                    // Pepare Wiring_check command when all wires not broke
                    tx_packet.PacketSrc = UART_Packet;
                    tx_packet.CMD = STHERM::Check_Wiring;
                    tx_packet.ACK = STHERM::ERROR_NO;
                    tx_packet.SID = 0x01;
                    tx_packet.DataLen = 0;

                    // Prepare packet to send request.
                    uint8_t tx_buff[256];
                    uint16_t size = UtilityHelper::setSIOTxPacket(tx_buff, tx_packet);

                    QByteArray packet = QByteArray::fromRawData(reinterpret_cast<char *>(tx_buff),
                                                                size);

                    LOG_DEBUG(
                        "***** Ti  - ERROR_WIRING_NOT_CONNECTED: Send Check_Wiring command *****");
                    tiConnection->sendRequest(packet);
                }

                break;
                case STHERM::ERROR_COULD_NOT_SET_RELAY:
                    LOG_DEBUG("ERROR_COULD_NOT_SET_RELAY");
                    if (rxPacket.DataLen) {
                        LOG_DEBUG(QString("%0\n relay indx ").arg(rxPacket.DataLen));
                        for (int k = 0; k < rxPacket.DataLen; k++) {
                            LOG_DEBUG(QString("%0").arg(rxPacket.DataArray[k]));
                        }
                    }

                    break;
                }
                break;
            }

        } break;

        case STHERM::Send_packet: {
            LOG_DEBUG("***** Ti  - Start Send_packet *****");
            LOG_DEBUG("***** Ti  - Send_packet finished *****");

        } break;

        case STHERM::GetRelaySensor: {
            LOG_DEBUG("***** Ti  - Start GetRelaySensor *****");
            LOG_DEBUG("***** Ti  - GetRelaySensor finished *****");

        } break;
        case STHERM::Check_Wiring: {
            LOG_DEBUG("***** Ti  - Start Check_Wiring *****");

//            wait_for_wiring_check = false;
            if (rxPacket.DataLen == WIRING_IN_CNT) {

                // Check: Update model
                for (int var = 0; var < WIRING_IN_CNT; ++var) {
                    mWiringState.append(rxPacket.DataArray[var]);
                }

                if (mWiringState.contains(WIRING_BROKEN)) {
                    emit alert(STHERM::LVL_Emergency, STHERM::Alert_wiring_not_connected);
                    LOG_DEBUG("Check_Wiring : Wiring is disrupted");
                } else {

                    // Pepare SetRelay command when all wires not broke
                    tx_packet.PacketSrc = UART_Packet;
                    tx_packet.CMD = STHERM::SetRelay;
                    tx_packet.ACK = STHERM::ERROR_NO;
                    tx_packet.SID = 0x01;
                    tx_packet.DataLen = qMin(RELAY_OUT_CNT, relays_in.count());

                    // Add relays_in elements into DataArray of packet
                    for (int i = 0; i < RELAY_OUT_CNT && i < relays_in.count(); i++) {
                        tx_packet.DataArray[i] = relays_in[i];
                    }

                    // Prepaare packet to send request.
                    uint8_t tx_buff[256];
                    uint16_t size = UtilityHelper::setSIOTxPacket(tx_buff, tx_packet);

                    QByteArray packet = QByteArray::fromRawData(reinterpret_cast<char *>(tx_buff),
                                                                size);

                    LOG_DEBUG("***** Ti  - Check_Wiring: Send SetRelay command *****");
                    tiConnection->sendRequest(packet);
                }

            } else {
                LOG_DEBUG("***** Ti  - Check_Wiring Error *****");
            }
            LOG_DEBUG("***** Ti  - Finished: Check_Wiring *****");

        } break;
        case STHERM::GetInfo: {
            LOG_DEBUG("***** Ti  - Start GetInfo *****");

            STHERM::SIOPacket tp;
            tp.PacketSrc = UART_Packet;
            tp.CMD = STHERM::Get_addr;
            tp.ACK = STHERM::ERROR_NO;
            tp.SID = 0x01;
            tp.DataLen = 0;
            indx_rev = 0;

            // TODO: uncomment later
            /* for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++)
            {
                TI_HW.push_back(static_cast<char>(rxPacket.DataArray[indx_rev]));
            }
            ++indx_rev;
            for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++)
            {
                TI_SW.push_back(static_cast<char>(rxPacket.DataArray[indx_rev]));
            }
            */

            // Prepare to send txPacket
            uint8_t tpBuff[256];
            uint16_t size = UtilityHelper::setSIOTxPacket(tpBuff, tp);

            QByteArray packet = QByteArray::fromRawData(reinterpret_cast<char *>(tpBuff),
                                                        size);
            LOG_DEBUG("***** Ti  - Get_addr packet sent to ti *****");
            tiConnection->sendRequest(packet);

            LOG_DEBUG("***** Ti  - Finished: GetInfo *****");

        } break;
        case STHERM::Get_addr: {
            LOG_DEBUG("***** Ti  - Start Get_addr *****");
            mMainDevice.address = *(uint32_t *)(rxPacket.DataArray);
            LOG_DEBUG("***** Ti  - Finished: Get_addr *****");

        } break;
        case STHERM::GET_DEV_ID: {
            // Check: loop detected in send GET_DEV_ID request.
            LOG_DEBUG("***** Ti  - Start GET_DEV_ID *****");
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = STHERM::GET_DEV_ID;
            tx_packet.ACK = STHERM::ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;
            // uncomment later
            /*
            if ((sizeof(dev_id) + TI_HW.length() + 1 + TI_SW.length() + 1 + NRF_HW.length() + 1 + NRF_SW.length() + 1 + sizeof(Daemon_Version)) > sizeof(tx_packet.DataArray))
            {
                syslog(LOG_INFO, "ERROR VERSION LENGTH\n");
                break;
            } */

            // CHECK
            memcpy(tx_packet.DataArray, mDeviceID.toUtf8(), sizeof(mDeviceID.toUtf8()));
            tx_packet.DataLen = sizeof(mDeviceID.toUtf8());

            // uncomment later
            /*
            memcpy(tx_packet.DataArray + tx_packet.DataLen, NRF_HW.c_str(), NRF_HW.length() + 1);
            tx_packet.DataLen += NRF_HW.length() + 1;
            memcpy(tx_packet.DataArray + tx_packet.DataLen, NRF_SW.c_str(), NRF_SW.length() + 1);
            tx_packet.DataLen += NRF_SW.length() + 1;
            memcpy(tx_packet.DataArray + tx_packet.DataLen, Daemon_Version, sizeof(Daemon_Version));
            tx_packet.DataLen += sizeof(Daemon_Version);
            rf_tx_buff_size = Set_SIO_TxPacket(rf_tx_buff, tx_packet);
            write(fds[0].fd, rf_tx_buff, rf_tx_buff_size);
            tx_packet.DataLen = 0;
            */

            LOG_DEBUG("***** Ti  - Finished: GET_DEV_ID *****");
        } break;
        case STHERM::feed_wtd:

            break;
        default:
            LOG_DEBUG(QString("ERROR_CMD %0").arg(rxPacket.CMD));
            break;
        }
    } else {
        LOG_DEBUG(QString("Ti - Calculated CRC and CRC are distinct. Calculated CRC:%0 CRC:%1").arg(inc_crc_ti).arg(rxPacket.CRC));

        STHERM::SIOPacket tp;
        tp.PacketSrc = UART_Packet;
        tp.CMD = rxPacket.CMD;
        tp.ACK = STHERM::ERROR_CRC;
        tp.SID = 0x01;
        tp.DataLen = 0;

        // Prepare to send ERROR_CRC packet
        uint8_t tpBuff[256];
        uint16_t size = UtilityHelper::setSIOTxPacket(tpBuff, tp);

        QByteArray packet = QByteArray::fromRawData(reinterpret_cast<char *>(tpBuff),
                                                    size);
        LOG_DEBUG("***** Ti  - ERROR_CRC packet sent to ti *****");
        tiConnection->sendRequest(packet);
    }

}

void DeviceIOController::checkMainDataAlert(const STHERM::AQ_TH_PR_vals &values)
{
    if (values.temp > AQ_TH_PR_thld.temp_high) {
        emit alert(STHERM::LVL_Emergency, STHERM::Alert_temp_high);

    }  else if (values.temp < AQ_TH_PR_thld.temp_low) {
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

    } else if (values.iaq > AQ_TH_PR_thld.iaq_high) {
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

    STHERM::SensorConfigThresholds th;
    checkSensorThreshold(th);
    sensorsThresholds.append(th);


    return sensorsThresholds;
}

void DeviceIOController::checkSensorThreshold(STHERM::SensorConfigThresholds &threshold)
{
    // The commented lines must be check,
    switch (threshold.sens_type)
    {
    case STHERM::SNS_temperature:
        if (threshold.max_alert_value > 127)
            threshold.max_alert_value = 127;
        //        throlds_aq.temp_high = static_cast<uint8_t>(i.max_alert_value);
        if (threshold.min_alert_value < -128)
            threshold.min_alert_value = -128;
        //        throlds_aq.temp_low = static_cast<uint8_t>(i.min_alert_value);
        break;
    case STHERM::SNS_humidity:
        if (threshold.max_alert_value > 100)
            threshold.max_alert_value = 100;
        //        throlds_aq.humidity_high = static_cast<uint8_t>(i.max_alert_value);
        if (threshold.min_alert_value < 0)
            threshold.min_alert_value = 0;
        //        throlds_aq.humidity_low = static_cast<uint8_t>(i.min_alert_value);
        break;
    case STHERM::SNS_co2:
        //        throlds_aq.c02_high = i.max_alert_value;
        break;
    case STHERM::SNS_etoh:
        if (threshold.max_alert_value > 127)
            threshold.max_alert_value = 127;
        //        throlds_aq.etoh_high = static_cast<uint8_t>(i.max_alert_value);
        break;
    case STHERM::SNS_iaq:
        if (threshold.max_alert_value > 5)
            threshold.max_alert_value = 5;
        //        throlds_aq.iaq_high = static_cast<uint8_t>(i.max_alert_value);
        break;
    case STHERM::SNS_Tvoc:
        if (threshold.max_alert_value > 127)
            threshold.max_alert_value = 127;
        //        throlds_aq.Tvoc_high = static_cast<uint8_t>(i.max_alert_value);
        break;
    case STHERM::SNS_pressure:
        //        throlds_aq.pressure_high = i.max_alert_value;
        break;
    default:
        break;
    }
}


QList<STHERM::DeviceType> DeviceIOController::getPairedSensors()
{
    QList<STHERM::DeviceType> sensors;

    STHERM::DeviceType deviceType;
//    deviceType.address = get external_sensor_id from sensors
//    deviceType.type = get external_sensor_type from sensors
//    deviceType.paired = true;


    return sensors;
}

bool DeviceIOController::addPendingSensor(STHERM::DeviceType inc)
{
    return true;
}

void DeviceIOController::getDeviceID()
{
    mDeviceID = QString();
}
