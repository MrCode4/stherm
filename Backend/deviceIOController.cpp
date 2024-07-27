#include "deviceIOController.h"

#include <QDateTime>
#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtConcurrent/QtConcurrent>
#include <ScreenSaverManager.h>

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

class DeviceIOPrivate
{
public:
    DeviceIOPrivate() {}

    //! Device id
    QString DeviceID;

    bool nRFWaitForResponse = false;
    bool wait_for_wiring_check = true;
    bool wiring_check_f = false;
    bool wait_relay_response = false;

    STHERM::SIOPacket SensorPacketBA;
    STHERM::SIOPacket TOFPacketBA;

    STHERM::ResponseTime Rtv;

    STHERM::AQ_TH_PR_thld throlds_aq;
    STHERM::AQ_TH_PR_vals aqthpr_dum_val;
    uint8_t cpIndex = 0;
    //    rgb_vals RGBm, RGBm_last;

    //! wirings and relays
    STHERM::RelayConfigs mRelaysIn;   // Fill from get_dynamic1, <relay_state> (0 OR 1)
    STHERM::RelayConfigs mRelaysOut;   // Fill from get_dynamic1, <relay_state> (0 OR 1)
    STHERM::RelayConfigs mRelaysOutLast;   // Fill from get_dynamic1, <relay_state> (0 OR 1)
    QList<uint8_t> WiringState;

    //! main device
    STHERM::DeviceType MainDevice;

    //! Paired devices
    int dev_count_indx;
    std::vector<STHERM::DeviceType> device_list;
    std::vector<STHERM::SensorTimeConfig> time_configs;
    std::vector<STHERM::SensorConfigThresholds> throlds;

    //! 0 normal, 1 adaptive
    uint8_t brightness_mode = 0;
    uint32_t luminosity = 255;
    uint32_t brightnessValue = 255;

    //! unknowns // TODO find unused ones and remove later
    bool pairing = false;
    bool last_pairing = false;
    uint64_t last_update_tick{};
    uint64_t last_update_tick_l{};
    int error_dynamic_counter{};

    qint64 lastTimeSensors = 0;
};

DeviceIOController::DeviceIOController(QObject *parent)
    : QObject{parent}
    , m_p(new DeviceIOPrivate)
{
    // move creating objects here
    m_tiConnection = new UARTConnection(TI_SERIAL_PORT, false, this);
    m_nRfConnection = new UARTConnection(NRF_SERIAL_PORT, false, this);
    m_gpioHandler4 = new GpioHandler(NRF_GPIO_4, this);
    m_gpioHandler5 = new GpioHandler(NRF_GPIO_5, this);

    initialize();

    m_backlightFactorUpdater.setInterval(1000);
    m_backlightFactorUpdater.setSingleShot(true);
    connect(&m_backlightFactorUpdater, &QTimer::timeout, this, [this]() {
        bool ok;
        double target = m_backlightFactorUpdater.property("target").toDouble(&ok);
        if (!ok)
            return;
        double diff = m_backlightFactorUpdater.property("diff").toDouble(&ok);
        if (!ok)
            return;
        m_backlightFactor += diff / 60;
        if (qAbs(m_backlightFactor - target) < qAbs(diff / 120)) {
            m_backlightFactor = target;
        } else  if (qAbs(diff) > 1E-3) {
            m_backlightFactorUpdater.start();
        }
        // when it reaches to the target stops and will not print anymore
        TRACE_CHECK(false) << "backlight factor updated to "  << m_backlightFactor << "with step " << diff / 20 << "and Target " << target;
    });
}

DeviceIOController::~DeviceIOController()
{
    m_wiring_timer.stop();
    m_wtd_timer.stop();
    m_nRF_timer.stop();
    m_adaptiveBrightness_timer.stop();

    stopReading();
    qWarning() << "Stopped Hvac";

    delete m_p;
}

void DeviceIOController::initialize()
{
    //! Get device id
    getDeviceID();

    m_p->brightnessValue = UtilityHelper::brightness();

    //! Prepare main device
    m_p->MainDevice.address = 0xffffffff; // this address is used in rf comms
    m_p->MainDevice.paired = STHERM::pair;
    m_p->MainDevice.type = STHERM::Main_dev;

    //! RTV
    m_p->Rtv.TP_internal_sesn_poll = 200; // 2 sec
    m_p->Rtv.TT_if_ack = 40;              // 10 min
    m_p->Rtv.TT_if_nack = 25;
    // TODO //    if (get_Config_list(m_p->time_configs))
    {
        // update RTV
        /*        for (auto i : time_configs)
        {
            switch (i.ext_sens_type)
            {
            case AQ_TH_PR:
                Rtv.TP_internal_sesn_poll = i.TP_internal_sesn_poll;
                Rtv.TT_if_ack = i.TT_if_ack;
                Rtv.TT_if_nack = i.TT_if_nack;
                break;
            default:
                break;
            }
        }*/
    }
    // else
    if (false) {
        qWarning() << "Error : get_Config_list";
    }

    //! update pairing devices
    //    if (!get_paired_list(m_p->device_list)) // send to ti
    {
        qWarning() << "Error: get_paired";
    }

    //! get thresholds
    //    if (get_thresholds_list(m_p->throlds))
    {
        for (auto i : m_p->throlds) {
            switch (i.sens_type) {
            case STHERM::SNS_temperature:
                if (i.max_alert_value > 127)
                    i.max_alert_value = 127;
                m_p->throlds_aq.temp_high = static_cast<uint8_t>(i.max_alert_value);
                if (i.min_alert_value < -128)
                    i.min_alert_value = -128;
                m_p->throlds_aq.temp_low = static_cast<uint8_t>(i.min_alert_value);
                break;
            case STHERM::SNS_humidity:
                if (i.max_alert_value > 100)
                    i.max_alert_value = 100;
                m_p->throlds_aq.humidity_high = static_cast<uint8_t>(i.max_alert_value);
                if (i.min_alert_value < 0)
                    i.min_alert_value = 0;
                m_p->throlds_aq.humidity_low = static_cast<uint8_t>(i.min_alert_value);
                break;
            case STHERM::SNS_co2:
                m_p->throlds_aq.c02_high = i.max_alert_value;
                break;
            case STHERM::SNS_etoh:
                if (i.max_alert_value > 127)
                    i.max_alert_value = 127;
                m_p->throlds_aq.etoh_high = static_cast<uint8_t>(i.max_alert_value);
                break;
            case STHERM::SNS_iaq:
                if (i.max_alert_value > 5)
                    i.max_alert_value = 5;
                m_p->throlds_aq.iaq_high = static_cast<uint8_t>(i.max_alert_value);
                break;
            case STHERM::SNS_Tvoc:
                if (i.max_alert_value > 127)
                    i.max_alert_value = 127;
                m_p->throlds_aq.Tvoc_high = static_cast<uint8_t>(i.max_alert_value);
                break;
            case STHERM::SNS_pressure:
                m_p->throlds_aq.pressure_high = i.max_alert_value;
                break;
            default:
                break;
            }
        }
    }
    if (false) //else
    {
        qWarning() << "Error: get_thresholds_list";
    }

    //    m_p->RGBm.red = 255;
    //    m_p->RGBm.blue = 255;
    //    m_p->RGBm.green = 255;
    //    m_p->RGBm.mode = LED_FADE;

    nrfConfiguration();
    tiConfiguration();

    TRACE << "Initialization done";
}

void DeviceIOController::nrfConfiguration()
{
    ///! initialize some structures

    STHERM::AQ_TH_PR_thld throldsAQ; // TODO getValues from settings?!

    m_p->SensorPacketBA = DataParser::prepareSIOPacket(STHERM::GetSensors);
    m_p->TOFPacketBA = DataParser::prepareSIOPacket(STHERM::GetTOF);

    ///! set initial configs

    ///! Send GetInfo request to initialize communication
    auto packet = DataParser::prepareSIOPacket(STHERM::GetInfo);
    m_nRF_queue.push(packet);

    ///! Send InitMcus request after GetInfo
    /// we can add this to a queue in constructor later and process the queue here
    STHERM::SIOPacket txPacket = DataParser::prepareSIOPacket(STHERM::InitMcus,
                                                              STHERM::UARTPacket,
                                                              {QVariant::fromValue(throldsAQ)});

    m_nRF_queue.push(txPacket);
    ///! to get the sensors value ASAP
    m_nRF_queue.push(m_p->SensorPacketBA);
}

void DeviceIOController::tiConfiguration()
{
    // Send GetInfo request
    auto packet = DataParser::prepareSIOPacket(STHERM::GetInfo);
    m_TI_queue.push(packet);

    //!TODO moving some data structures seems unnecessary
    qInfo() << QString("tack: %0 tnack : %1 senspt : %2")
                   .arg(m_p->Rtv.TT_if_ack)
                   .arg(m_p->Rtv.TT_if_nack)
                   .arg(m_p->Rtv.TP_internal_sesn_poll);

    ///! Send StartPairing request after GetInfo, TODO should we wait for reply?
    /// we can add this to a queue in constructor later and process the queue here
    /// this section is done different in daemon and passes multiple data to thread! which we do not need
    /// Set_limits and Set_time
    auto pairPacket = DataParser::prepareSIOPacket(STHERM::StartPairing);
    if (false) { // todo if there is anyrhing to pair
        m_TI_queue.push(pairPacket);
    }
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
    value = std::clamp(value, 5, 254);

    if (m_p->brightnessValue == value) {
        return true;
    }

    if (m_p->brightness_mode == 1) {
        emit adaptiveBrightness(value / 2.55);
    }

    m_p->brightnessValue = value;

    return UtilityHelper::setBrightness(m_p->brightnessValue);
}

void DeviceIOController::setTimeZone(int offset)
{
    UtilityHelper::setTimeZone(offset);
}

void DeviceIOController::createConnections()
{
    createNRF();
    createTIConnection();

    connect(&m_wtd_timer, &QTimer::timeout, this, &DeviceIOController::wtdExec);

    m_wtd_timer.setInterval(5000);
    m_wtd_timer.setSingleShot(false);
    m_wtd_timer.start();

    // The daemon regularly sends a "Check_Wiring" request every 600 seconds using a timer.
    // Inside the daemon, a thread waits for one second and counts up to 600. When the timer reaches 600,
    // the daemon sends the "Check_Wiring" request to Ti.
    // Note: Wait in getDynamic10 function in php, timer with tmr_cntr controls the "Check_Wiring" request.
    connect(&m_wiring_timer, &QTimer::timeout, this, &DeviceIOController::wiringExec);

    m_wiring_timer.setSingleShot(false);
    // m_wiring_timer.start(12000); // wiring is not valid in firmware anymore

    //! the sensors will get every 30 seonds or from gpio interrupts! timer will restart on interrupts
    connect(&m_nRF_timer, &QTimer::timeout, this, &DeviceIOController::nRFExec);
    m_nRF_timer.setInterval(30000);
    m_nRF_timer.setSingleShot(false);
    m_nRF_timer.start();

    //    start();

    connect(&m_adaptiveBrightness_timer, &QTimer::timeout, this, [this]() {
        int targetValue = m_p->luminosity;
        //! This will be brighter in minimum values!
        //! qRound(255.0 * std::sqrt(m_p->luminosity / 255.0));
        //! This will be darker in larger values!
        //! qRound(std::sqrt(m_p->luminosity));

        // clamp to range for better compare!
        targetValue = std::clamp(targetValue, 5, 254);

        if (targetValue == m_p->brightnessValue)
        {
            m_adaptiveBrightness_timer.stop();
            return;
        }

        int direction = targetValue > m_p->brightnessValue ? 1 : -1;
        setBrightness(m_p->brightnessValue + direction);
    });
    m_adaptiveBrightness_timer.setInterval(100);
}

void DeviceIOController::createSensor(QString name, QString id) {}

void DeviceIOController::createTIConnection()
{
    if (!m_tiConnection->startConnection(QSerialPort::Baud9600)) {
        LOG_DEBUG(QString("m_tiConnection startConnection failed"));
        return;
    }

    connect(m_tiConnection, &UARTConnection::sendData, this, [=](QByteArray data) {

        auto rxPacket = DataParser::deserializeData(data);
        bool trace = rxPacket.CMD != STHERM::feed_wtd;
        TRACE_CHECK(trace) << QString("Ti Response: %0").arg(data.toHex(' '));
        //        m_wtd_timer.start();

        TRACE_CHECK(trace) << QString("TI Response - CMD: %0").arg(rxPacket.CMD);
        processTIResponse(rxPacket);

        processTIQueue();
    });

    processTIQueue();
}

void DeviceIOController::createNRF()
{
    if (!m_nRfConnection->startConnection(QSerialPort::Baud9600)) {
        LOG_DEBUG(QString("m_nRfConnection startConnection failed"));
        return;
    }

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

    connect(m_nRfConnection, &UARTConnection::sendData, this, [=](QByteArray data) {
        TRACE_CHECK(false) << "NRF Response:   " << data;
        auto rxPacket = DataParser::deserializeData(data);
        TRACE_CHECK(false) << (QString("NRF Response - CMD: %0").arg(rxPacket.CMD));
        auto sent = m_nRF_queue.front();
        if (sent.CMD != rxPacket.CMD)
            qWarning() << "NRF RESPONSE IS ANOTHER CMD" << sent.CMD << rxPacket.CMD << m_nRF_queue.size();
        processNRFResponse(rxPacket, sent);
        m_nRfConnection->setProperty("busy", false);
        if (!m_nRF_queue.empty())
            m_nRF_queue.pop();
        processNRFQueue();
    });

    m_nRfConnection->setProperty("busy", false);
    processNRFQueue();

    if (m_gpioHandler4->startConnection() ) {
        connect(m_gpioHandler4, &GpioHandler::readyRead, this, [=](QByteArray data) {
            auto time = QDateTime::currentMSecsSinceEpoch();
            if (time - m_p->lastTimeSensors < 1000)
                return;
            if (data.length() == 2 && data.at(0) == '0') {
                m_p->lastTimeSensors = time;
                m_nRF_queue.push(m_p->SensorPacketBA);
                bool processed = processNRFQueue(STHERM::SIOCommand::GetSensors);
                TRACE_CHECK(false) << "request for gpio 4" << processed;
                // check after tiemout if no other request sent
                m_nRF_timer.start();
            }
        });
    } else {
        qWarning() << "GPIO 4 failed to connect" ;
    }

    if (m_gpioHandler5->startConnection()) {
        connect(m_gpioHandler5, &GpioHandler::readyRead, this, [=](QByteArray data) {
            if (data.length() == 2 && data.at(0) == '0') {
                bool debug = false;
                if (m_nRF_queue.empty() || m_nRF_queue.back().CMD != STHERM::SIOCommand::GetTOF) {
                    m_nRF_queue.push(m_p->TOFPacketBA);
                    debug = true;
                }
                bool processed = processNRFQueue(STHERM::SIOCommand::GetTOF);
                TRACE_CHECK(debug) << "request for gpio 5" << processed;
            }
        });
    } else {
        qWarning() << "GPIO 5 failed to connect" ;
    }
}

void DeviceIOController::stopReading()
{
    m_nRfConnection->disconnectDevice();
    m_gpioHandler4->closeFile();
    m_gpioHandler5->closeFile();
    m_tiConnection->disconnectDevice();
}

void DeviceIOController::updateTiDevices()
{
    // Temperature sensor

    // humidity sensor

    // Tof sensor

    // Ambient sensor

    // CO2 sensor

    qDebug() << Q_FUNC_INFO << __LINE__ << "Device count:   " << m_p->DeviceID.size();
}

bool DeviceIOController::update_nRF_Firmware()
{
    TRACE_CHECK(true) << "sending get into dfu:"
                       << (m_nRfConnection && m_nRfConnection->isConnected());

    auto packet = DataParser::prepareSIOPacket(STHERM::SIOCommand::GetIntoDFU,
                                               STHERM::PacketType::UARTPacket);
    m_nRF_queue.push(packet);

    auto result = processNRFQueue(STHERM::SIOCommand::GetIntoDFU);
    TRACE_CHECK(result) << "sending get into dfu failed or waiting in queue";
    return result;
}

void DeviceIOController::updateRelays(STHERM::RelayConfigs relays)
{
    //! In daemon: main.cpp: Line 1495 to 1518

    // check
    if (m_p->mRelaysIn == relays)
        return;

    m_p->mRelaysIn = relays;

    sendRelays();
}

bool DeviceIOController::testRelays(QVariantList relaysData)
{
    TRACE_CHECK(false) << "testRelays request with data:" << relaysData
                       << (m_tiConnection && m_tiConnection->isConnected());

    if (relaysData.size() == 12) {
        STHERM::RelayConfigs  relays;
        // 0 and 1 are R & C
        relays.g = relaysData[2].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.y1 = relaysData[3].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.y2 = relaysData[4].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.acc2 = relaysData[5].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.w1 = relaysData[6].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.w2 = relaysData[7].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.w3 = relaysData[8].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.o_b = relaysData[9].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.acc1p = relaysData[10].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        relays.acc1n = relaysData[11].toBool() ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
        auto packet = DataParser::prepareSIOPacket(STHERM::SIOCommand::SetRelay,
                                                   STHERM::PacketType::UARTPacket,
                                                   {QVariant::fromValue(relays)});

        m_TI_queue.push(packet);
        auto result = processTIQueue();
        TRACE_CHECK(false) << "send testRelays request" << result;
        return result;
    } else {
        qWarning() << "testRelays not sent: data is empty or not consistent" << relaysData;
    }

    return false;
}

void DeviceIOController::sendRelays()
{
    STHERM::SIOPacket packet;

    if (!checkRelayVaidation()) {
        TRACE << "Send Check_Wiring request.";
        // Prepare Check_Wiring packet
        packet = DataParser::prepareSIOPacket(STHERM::Check_Wiring);

    } else {
        TRACE << "Send SetRelay request.";
        // Prepare Set relay packet
        packet = DataParser::prepareSIOPacket(STHERM::SetRelay, STHERM::UARTPacket, {QVariant::fromValue(m_p->mRelaysIn)});
    }

    m_TI_queue.push(packet);
    processTIQueue();
}


bool DeviceIOController::setBacklight(QVariantList data)
{
    TRACE_CHECK(false) << "sending setBacklight request with data:" << data
                       << (m_nRfConnection && m_nRfConnection->isConnected());

    if (data.size() == 5) {
        if (m_nRF_queue.empty() || m_nRF_queue.back().CMD != STHERM::SIOCommand::SetColorRGB) {
            auto packet = DataParser::prepareSIOPacket(STHERM::SIOCommand::SetColorRGB,
                                                       STHERM::PacketType::UARTPacket,
                                                       data);
            m_nRF_queue.push(packet);
        } else  {
            auto last = m_nRF_queue.back();
            bool on = data[4].toBool();
            last.DataArray[0] = on ? std::clamp(data[0].toInt(), 0, 255) : 0;
            last.DataArray[1] = on ? std::clamp(data[1].toInt(), 0, 255) : 0;
            last.DataArray[2] = on ? std::clamp(data[2].toInt(), 0, 255) : 0;
            last.DataArray[3] = 255;
            last.DataArray[4] = data[3].toInt();
        }

        auto result = processNRFQueue(STHERM::SIOCommand::SetColorRGB);
        TRACE_CHECK(result) << "send setBacklight request failed or waiting in queue";
        return true;
    } else {
        qWarning() << "backlight not sent: data is empty or not consistent";
    }

    return false;
}

bool DeviceIOController::setFanSpeed(int speed)
{
    TRACE_CHECK(false) << "sending fan speed request with data:" << speed
                       << (m_nRfConnection && m_nRfConnection->isConnected());

    auto packet = DataParser::prepareSIOPacket(STHERM::SIOCommand::SetFanSpeed,
                                               STHERM::PacketType::UARTPacket,
                                               {speed});
    m_nRF_queue.push(packet);

    auto result = processNRFQueue(STHERM::SIOCommand::SetFanSpeed);
    TRACE_CHECK(result) << "send fan speed request failed or waiting in queue";
    return result;
}

bool DeviceIOController::setSettings(QVariantList data)
{
    if (data.size() <= 0) {
        qWarning() << "data sent is empty";
        return false;
    } else {
        if (data.size() != 4) {
            qWarning() << "data sent is not consistent";
            return false;
        }

        bool adaptive = data.last().toBool();
        m_p->brightness_mode = adaptive ? 1 : 0;

        TRACE_CHECK(adaptive) << "Adaptive enabled" << m_p->luminosity; // should not be in log as adaptive disabled for now

        if (setBrightness(adaptive ? m_p->luminosity :
                              qRound(data.first().toDouble() * 2.55)))
            return true;
        else
            return false;
    }
}

void DeviceIOController::setBrightnessTest(int brightness, bool test)
{
    if (test && !property("test").toBool()){
        setProperty("adaptive", m_p->brightness_mode);
        setProperty("brightness", m_p->brightnessValue);
    }
    setProperty("test", test);

    m_p->brightness_mode = test ? 0 : property("adaptive").toInt();
    brightness = test ? brightness : property("brightness").toInt();

    setBrightness(brightness);
}

void DeviceIOController::wtdExec()
{
    if (m_tiConnection && m_tiConnection->isConnected()) {
        auto packet = DataParser::prepareSIOPacket(STHERM::SIOCommand::feed_wtd);
        auto rsp = sendTIRequest(packet);

        if (rsp == false) {
            TRACE << "Ti heartbeat message send failed";
        } else {
            TRACE_CHECK(false) << "Ti heartbeat message sent";
        }

        TRACE_CHECK(false) << "Ti heartbeat message finished" << rsp;
    } else {
        TRACE << "start wtd failed not connected" << m_tiConnection;
    }
}

void DeviceIOController::wiringExec()
{
    TRACE << "start Check_Wiring" << (m_tiConnection && m_tiConnection->isConnected());

    if (m_tiConnection && m_tiConnection->isConnected()) {
        auto packet = DataParser::prepareSIOPacket(STHERM::SIOCommand::Check_Wiring);
        m_TI_queue.push(packet);
        auto rsp = processTIQueue();

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
    bool debug = false;
#ifdef DEBUG_MODE
    debug = true;
#endif

    TRACE_CHECK(debug) << "start NRF" << (m_nRfConnection && m_nRfConnection->isConnected());
    if (m_nRfConnection && m_nRfConnection->isConnected()) {
        TRACE_CHECK(debug) << "start GetSensors";

        m_p->lastTimeSensors = QDateTime::currentMSecsSinceEpoch();

        m_nRF_queue.push(m_p->SensorPacketBA);
        auto result = processNRFQueue(STHERM::SIOCommand::GetSensors);

        TRACE_CHECK(debug) << "GetSensors message finished" << result;
    }
}

void DeviceIOController::processNRFResponse(STHERM::SIOPacket rxPacket, const STHERM::SIOPacket &txPacket)
{
    // checksum the response data
    uint16_t inc_crc_nrf = UtilityHelper::crc16(rxPacket.DataArray, rxPacket.DataLen);

    // check data integridy
    if (inc_crc_nrf == rxPacket.CRC) {
        if (rxPacket.ACK == STHERM::ERROR_NO) {

            switch (rxPacket.CMD) {
            case STHERM::GetInfo: {
                int indx_rev = 0;

                NRF_HW = "";
                NRF_SW = "";

                // TODO: When NRF_HW.clear() called?
                for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++) {
                    NRF_HW.push_back(static_cast<char>(rxPacket.DataArray[indx_rev]));
                }

                ++indx_rev;
                for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++) {
                    NRF_SW.push_back(static_cast<char>(rxPacket.DataArray[indx_rev]));
                }

                TRACE << " NRF_HW :" << NRF_HW << " NRF_SW :" << NRF_SW;

                emit nrfVersionUpdated();

            } break;

            case STHERM::GetIntoDFU: {
                // send actual file to nRF
                int exitCode = QProcess::execute("/bin/bash", {"-c", "/usr/local/bin/update_fw_nrf_seamless.sh"});
                TRACE << exitCode;

                // sleep and wait for nRF to restart
                QThread::msleep(5000);

                // Send GetInfo request to re-initialize communication
                auto packet = DataParser::prepareSIOPacket(STHERM::GetInfo);
                m_nRF_queue.push(packet);

            } break;

            case STHERM::GetTOF: {
                uint16_t RangeMilliMeter;
                uint32_t Luminosity;
                // Read RangeMilliMeter and Luminosity
                // In OLD code: 1118-1132
                memcpy(&RangeMilliMeter, rxPacket.DataArray, sizeof(RangeMilliMeter));
                memcpy(&Luminosity,
                       rxPacket.DataArray + sizeof(RangeMilliMeter),
                       sizeof(Luminosity));

                QVariantMap resultMap;
                resultMap.insert(RangeMilliMeterKey, RangeMilliMeter);
                resultMap.insert(brightnessKey, Luminosity);
                emit tofDataReady(resultMap);

                checkTOFRangeValue(RangeMilliMeter);
                checkTOFLuminosity(Luminosity);

            } break;

            case STHERM::GetSensors: {
                m_p->lastTimeSensors = QDateTime::currentMSecsSinceEpoch();
                uint16_t RangeMilliMeter;
                uint16_t fanSpeed;
                uint32_t Luminosity;
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

                uint16_t etoh_temp;
                memcpy(&etoh_temp, rxPacket.DataArray + cpIndex, sizeof(etoh_temp));
                cpIndex += sizeof(etoh_temp);
                mainDataValues.etoh = etoh_temp / 100.0;
                TRACE_CHECK(false) <<(QString("mainDataValues.etoh: %0").arg(mainDataValues.etoh));

                uint16_t tvoc_temp;
                memcpy(&tvoc_temp, rxPacket.DataArray + cpIndex, sizeof(tvoc_temp));
                cpIndex += sizeof(tvoc_temp);
                mainDataValues.Tvoc = tvoc_temp / 1000.0;
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
                mainDataMap.insert(temperatreKey,     mainDataValues.temp);
                mainDataMap.insert(humidityKey,        mainDataValues.humidity);
                mainDataMap.insert(co2Key,             mainDataValues.c02);
                mainDataMap.insert(etohKey,            mainDataValues.etoh);
                mainDataMap.insert(TvocKey,            mainDataValues.Tvoc);
                mainDataMap.insert(iaqKey,             mainDataValues.iaq);
                mainDataMap.insert(pressureKey,        mainDataValues.pressure);
                mainDataMap.insert(RangeMilliMeterKey, RangeMilliMeter);
                mainDataMap.insert(brightnessKey,       Luminosity);
                mainDataMap.insert(fanSpeedKey,        fanSpeed);

                emit mainDataReady(mainDataMap);

                // todo
                //                if (!set_fan_speed_INFO(fan_speed)) {
                //                    LOG_DEBUG(QString("Error: setFanSpeed: (fan speed: %0)").arg(fan_speed));
                //                }

                checkTOFRangeValue(RangeMilliMeter);
                checkTOFLuminosity(Luminosity);

                checkMainDataAlert(mainDataValues, fanSpeed, Luminosity);

                // todo
                //                if (!setSensorData(main_dev, rx_packet.DataArray, rx_packet.DataLen)) {
                //                    LOG_DEBUG(QString("Error: setSensorData"));
                //                }
            } break;
            case STHERM::SetFanSpeed:
            {
                if (txPacket.CMD == STHERM::SetFanSpeed && txPacket.DataLen == 1) {
                    emit fanStatusUpdated(txPacket.DataArray[0] == 0);
                } else {
                    qWarning() << "incompatible response of setFanSpeed" << txPacket.CMD << txPacket.DataLen;
                }
            }
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

bool DeviceIOController::processNRFQueue(STHERM::SIOCommand cause)
{
    if (!m_nRfConnection || !m_nRfConnection->isConnected())
        return false;

    if (m_nRF_queue.empty()) {
        return false;
    }

    auto packet = m_nRF_queue.front();

    if (m_nRfConnection->property("busy").toBool()) {
        TRACE_CHECK(cause != STHERM::GetTOF) << cause << " not sent, busy with previous one" << packet.CMD << m_nRF_queue.size();
        return false;
    }

    m_nRfConnection->setProperty("busy", true);

    //! prepare for request
    uint8_t ThreadBuff[256];
    int ThreadSendSize = UtilityHelper::setSIOTxPacket(ThreadBuff, packet);
    auto packetBA = QByteArray::fromRawData(reinterpret_cast<char *>(ThreadBuff), ThreadSendSize);

    if (m_nRfConnection->sendRequest(packetBA)) {
        if (packet.CMD == STHERM::SIOCommand::SetFanSpeed) {
            TRACE << "CHECK for set fan speed";
        } else if (packet.CMD == STHERM::SIOCommand::GetTOF) {
            TRACE_CHECK(false) << "CHECK for TOF values";
        } else if (packet.CMD == STHERM::SIOCommand::GetSensors) {
            TRACE_CHECK(false) << "CHECK for Sensor values";
            m_p->lastTimeSensors = QDateTime::currentMSecsSinceEpoch();
        } else if (packet.CMD == STHERM::SIOCommand::SetColorRGB) {
            // TODO: blinking with data array [4]
            TRACE_CHECK(false) << "Data " << packet.DataArray[0] << " " << packet.DataArray[1] << " " << packet.DataArray[2];
            double backlightFactor = ((double)packet.DataArray[0] + (double)packet.DataArray[1] + (double)packet.DataArray[2]) / (3.0 * 255.0);
            TRACE_CHECK(false) << "backlight factor will be updated to " << backlightFactor;
            m_backlightFactorUpdater.setProperty("diff", backlightFactor - m_backlightFactor);
            m_backlightFactorUpdater.setProperty("target", backlightFactor);
            m_backlightFactorUpdater.start();
        }
        //        m_nRF_queue.pop(); // we pop it when the result reciever so we can confirm
        return true;
    } else {
        qWarning() << "nRF request packet not sent" << packet.CMD
                   << "queue size: " << m_nRF_queue.size();
    }

    return false;
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
    m_p->wait_relay_response = false;
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

                sendTIRequest(tx_packet);

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

                sendTIRequest(tx_packet);
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
                TRACE_CHECK(false) << "***** Ti  - Start SetRelay *****";
                m_p->mRelaysOutLast = m_p->mRelaysOut;
                emit relaysUpdated(m_p->mRelaysOutLast);

                TRACE_CHECK(false) << "***** Ti  - SetRelay finished *****";
            } else {
                switch (rxPacket.ACK) {
                case STHERM::ERROR_WIRING_NOT_CONNECTED: {
                    m_p->wait_for_wiring_check = true;
                    emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_wiring_not_connected);
                    LOG_DEBUG("ERROR_WIRING_NOT_CONNECTED");
                    LOG_DEBUG("~" + QString::number(rxPacket.DataLen));
                    // Pepare Wiring_check command when all wires not broke
                    auto packet = DataParser::prepareSIOPacket(STHERM::Check_Wiring);

                    LOG_DEBUG(
                        "***** Ti  - ERROR_WIRING_NOT_CONNECTED: Send Check_Wiring command *****");

                    sendTIRequest(packet);
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

            m_p->wait_for_wiring_check = false;
            if (rxPacket.DataLen == WIRING_IN_CNT) {

                // Check: Update model
                for (int var = 0; var < WIRING_IN_CNT; ++var) {
                    m_p->WiringState.append(rxPacket.DataArray[var]);
                }

                if (!checkRelayVaidation()) { // Broken a wire
                    emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_wiring_not_connected);
                    // TODO relays_in_l = relays_in;
                    LOG_DEBUG("Check_Wiring : Wiring is disrupted");
                } else {
                    auto packet = DataParser::prepareSIOPacket(STHERM::SetRelay, STHERM::UARTPacket, {QVariant::fromValue(m_p->mRelaysIn)});

                    LOG_DEBUG("***** Ti  - Check_Wiring: Send SetRelay command *****");

                    sendTIRequest(packet);
                }

            } else {
                LOG_DEBUG("***** Ti  - Check_Wiring Error *****");
            }
            LOG_DEBUG("***** Ti  - Finished: Check_Wiring *****");

        } break;
        case STHERM::GetInfo: {
            TRACE << "***** Ti  - Start GetInfo *****";

            STHERM::SIOPacket tp;
            tp.PacketSrc = UART_Packet;
            tp.CMD = STHERM::Get_addr;
            tp.ACK = STHERM::ERROR_NO;
            tp.SID = 0x01;
            tp.DataLen = 0;
            indx_rev = 0;

            for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++) {
                TI_HW.push_back(static_cast<char>(rxPacket.DataArray[indx_rev]));
            }

            ++indx_rev;
            for (; rxPacket.DataArray[indx_rev] != 0 && indx_rev < sizeof(rxPacket.DataArray); indx_rev++) {
                TI_SW.push_back(static_cast<char>(rxPacket.DataArray[indx_rev]));
            }

            TRACE << "TI_HW: " << TI_HW << " TI_SW: " << TI_SW;

            emit tiVersionUpdated();

            sendTIRequest(tp);

            TRACE << "***** Ti  - Get_addr packet sent to ti *****";

        } break;
        case STHERM::Get_addr: {
            LOG_DEBUG("***** Ti  - Start Get_addr *****");
            m_p->MainDevice.address = *(uint32_t *) (rxPacket.DataArray);
            TRACE << "address: " << m_p->MainDevice.address ;
            LOG_DEBUG("***** Ti  - Finished: Get_addr *****");

        } break;
        case STHERM::GET_DEV_ID: { // TODO: what is this?
            // Check: loop detected in send GET_DEV_ID request.
            LOG_DEBUG("***** Ti  - Start GET_DEV_ID *****");
            tx_packet.PacketSrc = UART_Packet;
            tx_packet.CMD = STHERM::GET_DEV_ID;
            tx_packet.ACK = STHERM::ERROR_NO;
            tx_packet.SID = 0x01;
            tx_packet.DataLen = 0;

            char dev_id[16]{0};
            if ((sizeof(dev_id) + TI_HW.length() + 1 + TI_SW.length() + 1 + NRF_HW.length() + 1 + NRF_SW.length() + 1 + sizeof(Daemon_Version)) > sizeof(tx_packet.DataArray))  {
                TRACE << "ERROR VERSION LENGTH";
                break;
            }

            // CHECK
            memcpy(tx_packet.DataArray, m_p->DeviceID.toUtf8(), sizeof(m_p->DeviceID.toUtf8()));
            tx_packet.DataLen = sizeof(m_p->DeviceID.toUtf8());

            // uncomment later
            memcpy(tx_packet.DataArray + tx_packet.DataLen, NRF_HW.toStdString().c_str(), NRF_HW.length() + 1);
            tx_packet.DataLen += NRF_HW.length() + 1;
            memcpy(tx_packet.DataArray + tx_packet.DataLen, NRF_SW.toStdString().c_str(), NRF_SW.length() + 1);
            tx_packet.DataLen += NRF_SW.length() + 1;
            memcpy(tx_packet.DataArray + tx_packet.DataLen, Daemon_Version, sizeof(Daemon_Version));
            tx_packet.DataLen += sizeof(Daemon_Version);

            m_TI_queue.push(tx_packet);
            processTIQueue();

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

        LOG_DEBUG("***** Ti  - ERROR_CRC packet sent to ti *****");
        sendTIRequest(tp);
    }
}

bool DeviceIOController::processTIQueue()
{
    if (!m_tiConnection || !m_tiConnection->isConnected()) {
        TRACE;
        return false;
    }

    if (m_TI_queue.empty()) {
        return false;
    }


    auto packet = m_TI_queue.front();

    TRACE_CHECK(packet.CMD != STHERM::feed_wtd) << "TI Request" << packet.CMD;

    if ((packet.CMD == STHERM::SetRelay || packet.CMD == STHERM::Check_Wiring)
        && m_p->wait_relay_response) {
        TRACE;
        return false;
    }

    if (sendTIRequest(packet)) {
        m_TI_queue.pop();
        return true;
    } else {
        qWarning() << "ti request packet not sent" << packet.CMD
                   << "queue size: " << m_TI_queue.size();
    }

    return false;
}

bool DeviceIOController::sendTIRequest(STHERM::SIOPacket txPacket)
{
    //    m_wtd_timer.start();

    if (txPacket.CMD == STHERM::SetRelay || txPacket.CMD == STHERM::Check_Wiring) {
        m_p->wait_relay_response = true;
    }

    // Store relays that must be updated based on the server's response to the current SetRelay request.
    if (txPacket.CMD == STHERM::SetRelay) {
        m_p->mRelaysOut = DataParser::getRelaysFromPacket(txPacket);
    }

    //! prepare for request
    uint8_t ThreadBuff[256];
    int ThreadSendSize = UtilityHelper::setSIOTxPacket(ThreadBuff, txPacket);
    auto packetBA = QByteArray::fromRawData(reinterpret_cast<char *>(ThreadBuff), ThreadSendSize);
    return m_tiConnection->sendRequest(packetBA);
}

void DeviceIOController::checkMainDataAlert(const STHERM::AQ_TH_PR_vals &values, const uint16_t &fanSpeed, const uint32_t luminosity)
{
    // Manage temperature alerts
    if (values.temp > m_p->throlds_aq.temp_high) {
        if (m_TemperatureAlertET.isValid() && m_TemperatureAlertET.elapsed() >= 15 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_temp_high,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_temp_high));
            m_TemperatureAlertET.restart();

        } else if (!m_TemperatureAlertET.isValid()) {
            m_TemperatureAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_temp_high);
        }

    } else if (values.temp < m_p->throlds_aq.temp_low) {
        if (m_TemperatureAlertET.isValid() && m_TemperatureAlertET.elapsed() >= 15 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_temp_low,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_temp_low));
            m_TemperatureAlertET.restart();

        } else if (!m_TemperatureAlertET.isValid()) {
            m_TemperatureAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_temp_low);
        }

    } else {
        m_TemperatureAlertET.invalidate();
    }

    // Manage humidity alerts
    if (values.humidity > m_p->throlds_aq.humidity_high) {
        if (m_HumidityAlertET.isValid() && m_HumidityAlertET.elapsed() >= 15 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_humidity_high,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_humidity_high));
            m_HumidityAlertET.restart();

        } else if (!m_HumidityAlertET.isValid()) {
            m_HumidityAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_humidity_high);
        }

    } else if (values.humidity < m_p->throlds_aq.humidity_low) {
        if (m_HumidityAlertET.isValid() && m_HumidityAlertET.elapsed() >= 15 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_humidity_low,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_humidity_low));
            m_HumidityAlertET.restart();

        } else if (!m_HumidityAlertET.isValid()) {
            m_HumidityAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_humidity_low);
        }

    } else {
        m_HumidityAlertET.invalidate();
    }

    // Manage fan alerts
    if (fanSpeed > m_p->throlds_aq.fan_high) {
        if (m_FanAlertET.isValid() && m_FanAlertET.elapsed() >= 5 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_fan_High,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_fan_High));
            m_FanAlertET.restart();

        } else if (!m_FanAlertET.isValid()) {
            m_FanAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_fan_High);

        }

    } else if (fanSpeed < m_p->throlds_aq.fan_low) {
        if (m_FanAlertET.isValid() && m_FanAlertET.elapsed() >= 5 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_fan_low,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_fan_low));
            m_FanAlertET.restart();

        } else if (!m_FanAlertET.isValid()) {
            m_FanAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_fan_low);

        }

    } else {
        m_FanAlertET.invalidate();
    }

    // Manage light alerts
    if (luminosity < m_p->throlds_aq.light_low) {
        if (m_LightAlertET.isValid() && m_LightAlertET.elapsed() >= 24 * 60 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_Light_Low,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_Light_Low));
            m_LightAlertET.restart();

        } else if (!m_LightAlertET.isValid()) {
            m_LightAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_Light_Low);

        }

    } else if (luminosity > m_p->throlds_aq.light_high) {
        if (m_LightAlertET.isValid() && m_LightAlertET.elapsed() >= 24 * 60 * 60 * 1000) {
            emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_Light_High,
                       STHERM::getAlertTypeString(AppSpecCPP::Alert_Light_High));
            m_LightAlertET.restart();

        } else if (!m_LightAlertET.isValid()) {
            m_LightAlertET.start();
            TRACE_CHECK(false) << STHERM::getAlertTypeString(AppSpecCPP::Alert_Light_High);
        }

    } else {
        m_LightAlertET.invalidate();
    }

    if (values.pressure > m_p->throlds_aq.pressure_high) {
        emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_pressure_high,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_pressure_high));
    }

    if (values.c02 > m_p->throlds_aq.c02_high) {
        emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_c02_high,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_c02_high));

    } else if (values.c02 < m_p->throlds_aq.c02_low) {
        emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_c02_low,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_c02_low));

    }

    if (values.Tvoc > m_p->throlds_aq.Tvoc_high * 1000) {
        emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_Tvoc_high,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_Tvoc_high));

    }

    if (values.etoh > m_p->throlds_aq.etoh_high * 100) {
        emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_etoh_high,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_etoh_high));

    }

    if (values.iaq > m_p->throlds_aq.iaq_high) {
        emit alert(STHERM::LVL_Emergency, AppSpecCPP::Alert_iaq_high,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_iaq_high));

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
    //    TODO get and if not successful out warning
    m_p->DeviceID = QString();
}

bool DeviceIOController::checkRelayVaidation()
{
    // wiring check is not valid in firmware anymore!
    return true;

    if (m_p->WiringState.count() < WIRING_IN_CNT)
        return false;

    int i = 0;

    bool isNotValid = (m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.g     == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.y1    == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.y2    == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.y3    == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.acc2  == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.w1    == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.w2    == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.w3    == STHERM::ON ||
                       m_p->WiringState.at(i++) == STHERM::Broken && m_p->mRelaysIn.o_b   == STHERM::ON ||
                       m_p->WiringState.at(i)   == STHERM::Broken && m_p->mRelaysIn.acc1n == STHERM::ON);

    return !isNotValid;
}

void DeviceIOController::checkTOFRangeValue(uint16_t range_mm)
{
    TRACE_CHECK(false) << (QString("RangeMilliMeter (%0)").arg(range_mm));

    // TOF sensor activate display when distance < 1 meter and time > 1 second (handled in firmware)
    if (range_mm > 60 && range_mm <= TOF_IRQ_RANGE) {
        if (auto manager = ScreenSaverManager::instance()) {
            manager->restart();
        }
    }
}

void DeviceIOController::checkTOFLuminosity(uint32_t luminosity)
{
    TRACE_CHECK(m_p->brightness_mode == 1) << (QString("Luminosity (%1)").arg(luminosity)) <<
        m_p->brightness_mode << m_adaptiveBrightness_timer.isActive();
    m_p->luminosity = luminosity;  // we can smooth this as well if changes too much
    if (m_p->brightness_mode == 1) {
        if (!m_adaptiveBrightness_timer.isActive())
            m_adaptiveBrightness_timer.start();
    }
}

QString DeviceIOController::getTI_SW() const
{
    return TI_SW;
}

QString DeviceIOController::getTI_HW() const
{
    return TI_HW;
}

QString DeviceIOController::getNRF_SW() const
{
    return NRF_SW;
}

QString DeviceIOController::getNRF_HW() const
{
    return NRF_HW;
}

double DeviceIOController::backlightFactor()
{
    return m_backlightFactor;
}
