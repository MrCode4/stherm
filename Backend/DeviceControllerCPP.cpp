#include "DeviceControllerCPP.h"

#include "LogHelper.h"

//! Set CPU governer in the zeus base system
//! It is strongly dependent on the kernel.
inline void setCPUGovernor(QString governer) {
#ifdef __unix__
    QDir cpuDir("/sys/devices/system/cpu/");
    QStringList cpuList = cpuDir.entryList(QStringList() << "cpu[0-9]*");

    TRACE << "CPU List: =" << cpuList;

    foreach (const QString& cpu, cpuList) {
        QString governorFile = QString("/sys/devices/system/cpu/%1/cpufreq/scaling_governor").arg(cpu);
        QFile file(governorFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << governer; // Set CPU governor
            file.close();
            TRACE << "Set CPU" << cpu << "governor to " << governer;
        } else {
            TRACE << "Failed to set CPU" << cpu << "governor to performance";
        }
    }
#endif
}

DeviceControllerCPP* DeviceControllerCPP::sInstance = nullptr;

DeviceControllerCPP* DeviceControllerCPP::instance()
{
    return sInstance;
}

/* ************************************************************************************************
 * Constructors & Destructor
 * ************************************************************************************************/
DeviceControllerCPP::DeviceControllerCPP(QObject *parent)
    : QObject(parent)
    , _deviceIO(new DeviceIOController(this))
    , _deviceAPI(new DeviceAPI(this))
    , mSystemSetup(nullptr)
    , m_scheme(new Scheme(_deviceAPI, this))
{

    m_system = _deviceAPI->system();

    // todo: initialize with proper value
    mBacklightModelData = QVariantList();

    // Update backlight
    mBacklightTimer.setTimerType(Qt::PreciseTimer);
    mBacklightTimer.setSingleShot(true);
    connect(&mBacklightTimer, &QTimer::timeout, this, [this]() {
        auto colorData = mBacklightTimer.property("color").value<QVariantList>();
        TRACE << "restoring color with timer " << colorData;
        setBacklight(colorData, true);
    });

    // TODO should be loaded later for accounting previous session
    mDeltaTemperatureIntegrator = 0;

    QVariantMap mainDataMap;
    mainDataMap.insert("temperature",     0);
    mainDataMap.insert("humidity",        0);
    mainDataMap.insert("co2",             0);
    mainDataMap.insert("etoh",            0);
    mainDataMap.insert("Tvoc",            0);
    mainDataMap.insert("iaq",             0);
    mainDataMap.insert("pressure",        0);
    mainDataMap.insert("RangeMilliMeter", 0);
    mainDataMap.insert("brighness",       0);
    mainDataMap.insert("fanSpeed",        0);
    setMainData(mainDataMap);

    mNightModeTimer.setTimerType(Qt::PreciseTimer);
    mNightModeTimer.setInterval(5000 * 60);
    mNightModeTimer.setSingleShot(true);
    connect(&mNightModeTimer, &QTimer::timeout, this, [this]() {
        _deviceIO->setFanSpeed(0);
    });

    mNightModeLogTimer.setTimerType(Qt::PreciseTimer);
    mNightModeLogTimer.setInterval(10000);
    connect(&mNightModeLogTimer, &QTimer::timeout, this, [this]() {
        TRACE << "---------------------- Start Night Mode Log ----------------------";

        m_system->cpuInformation();
        TRACE << "Delta Correction: " << deltaCorrection() <<
            "- Delta Temperature Integrator: " << mDeltaTemperatureIntegrator <<
            "- backlightFactor: " << _deviceIO->backlightFactor();


        TRACE << "Brightness: " << UtilityHelper::brightness();

        TRACE << "Raw Temperature: " << mRawTemperature;

        TRACE << "---------------------- End Night Mode Log ----------------------";
    });

    mBacklightPowerTimer.setTimerType(Qt::PreciseTimer);
    mBacklightPowerTimer.setSingleShot(false);
    mBacklightPowerTimer.setInterval(1000);
    connect(&mBacklightPowerTimer, &QTimer::timeout, this, [this]() {
        mDeltaTemperatureIntegrator *= TEMPERATURE_INTEGRATOR_DECAY_CONSTANT;
        mDeltaTemperatureIntegrator += _deviceIO->backlightFactor();
    });
    mBacklightPowerTimer.start();

    connect(_deviceIO, &DeviceIOController::mainDataReady, this, [this](QVariantMap data) {
        setMainData(data);
    });

    // Update nrf version
    connect(_deviceIO, &DeviceIOController::nrfVersionUpdated, this, [this]() {
        emit nrfVersionChanged();

        TRACE << "VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVvvvvvv" << getNRF_SW();
        if (getNRF_SW() != "01.10-RC1") {
            TRACE << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxx";
            QTimer::singleShot(10000, this, [this]() {m_system->updateFirmware();});
        }
    });

    // Update ti version
    connect(_deviceIO, &DeviceIOController::tiVersionUpdated, this, [this]() {
        emit tiVersionChanged();
    });

    connect(_deviceIO, &DeviceIOController::tofDataReady, this, [this](QVariantMap data) {
        for (const auto &pair : data.toStdMap()) {
            _mainData.insert(pair.first, pair.second);
        }
    });

    connect(m_scheme, &Scheme::changeBacklight, this, [this](QVariantList color, QVariantList afterColor) {

        TRACE_CHECK(false) << "Update backlight." << color << afterColor << mBacklightModelData;

        if (mBacklightTimer.isActive())
            mBacklightTimer.stop();

        if (color.isEmpty()) {
            TRACE << "restoring color with force " << mBacklightModelData;
            setBacklight(mBacklightModelData, true);
            return;
        }

        setBacklight(color, true);

        if (afterColor.isEmpty()) {
            afterColor = mBacklightModelData;
        }

        // Back to last backlight after secs seconds
        mBacklightTimer.start(5 * 1000);
        mBacklightTimer.setProperty("color", afterColor);
    });

    connect(m_scheme, &Scheme::updateRelays, this, [this](STHERM::RelayConfigs relays) {
        _deviceIO->updateRelays(relays);
    });

    if (m_system) {
        connect(m_system, &NUVE::System::systemUpdating, this, [this]() {
            m_scheme->moveToUpdatingMode();
        });
    }

    connect(_deviceIO,
            &DeviceIOController::alert,
            this,
            [this](STHERM::AlertLevel alertLevel,
                   STHERM::AlertTypes alertType,
                   QString alertMessage) {
                emit alert(alertLevel, alertType, alertMessage);
            });


    //! Set sInstance to this
    if (!sInstance) {
        sInstance = this;
    }
}

DeviceControllerCPP::~DeviceControllerCPP() {}


bool DeviceControllerCPP::setBacklight(QVariantList data, bool isScheme)
{
    bool success = _deviceIO->setBacklight(data);

    if (success && !isScheme) {
        mBacklightModelData = data;
    }

    return success;
}

//! TODO
//! Handle the CPU frequency or governor will be set to minimum speed level.
//! Handle other power limiting functions
void DeviceControllerCPP::nightModeControl(bool start)
{
    if (start) {
        setCPUGovernor("powersave");
        mNightModeTimer.start();
        mNightModeLogTimer.start();
        
        m_system->cpuInformation();	

    } else {
        mNightModeTimer.stop();
        mNightModeLogTimer.stop();
        setCPUGovernor("ondemand");
        _deviceIO->setFanSpeed(16); //100 / 7
    }

}

bool DeviceControllerCPP::setSettings(QVariantList data)
{
    return _deviceIO->setSettings(data);
}

void DeviceControllerCPP::setVacation(const double min_Temperature, const double max_Temperature,
                                      const double min_Humidity, const double max_Humidity)
{
    // Prepare vacation struct
    STHERM::Vacation vacation;
    vacation.minimumTemperature = min_Temperature;
    vacation.maximumTemperature = max_Temperature;
    vacation.minimumHumidity    = min_Humidity;
    vacation.maximumHumidity    = max_Humidity;

    // Update vacations in scheme
    if (m_scheme)
        m_scheme->setVacation(vacation);
}

void DeviceControllerCPP::setRequestedTemperature(const double temperature)
{
    if (m_scheme)
        m_scheme->setSetPointTemperature(temperature);
}

void DeviceControllerCPP::setRequestedHumidity(const double humidity)
{
    if (m_scheme)
        m_scheme->setRequestedHumidity(humidity);
}

bool DeviceControllerCPP::setTestRelays(QVariantList data)
{
    return _deviceIO->testRelays(data);
}

void DeviceControllerCPP::sendRelaysBasedOnModel()
{
    _deviceIO->sendRelays();
}

QString DeviceControllerCPP::getTI_SW() const
{
    return _deviceIO->getTI_SW();
}

QString DeviceControllerCPP::getTI_HW() const
{
    return _deviceIO->getTI_HW();
}

QString DeviceControllerCPP::getNRF_SW() const
{
    return _deviceIO->getNRF_SW();
}

QString DeviceControllerCPP::getNRF_HW() const
{
    return _deviceIO->getNRF_HW();
}

//! runDevice in Hardware.php
void DeviceControllerCPP::startDevice()
{
    //! todo: move to constructor later
    _deviceIO->createConnections();

    // Start by calling runDevice, which will load and populate the device config
    _deviceAPI->runDevice();

    int startMode = getStartMode();

    emit startModeChanged(startMode);

    // Start with delay to ensure the model loaded.
    // will be loaded always, but should be OFF in iniial setup mode as its default is OFF
    QTimer::singleShot(5000, this, [this]() {
        TRACE << "starting scheme";
        m_scheme->restartWork();
    });

    if (startMode == 0) {
        startTestMode();

        // if test mode returns, when the other codes should be run? after finishing test mode? //TODO
        return;
    }

    if (!checkSN()){
        TRACE << "INITAIL SETUP";
        return;
    }

    checkUpdateMode();
}

void DeviceControllerCPP::stopDevice()
{
    _deviceIO->stopReading();
    m_scheme->stop();
}

void DeviceControllerCPP::setActivatedSchedule(ScheduleCPP *schedule)
{
    if (m_scheme)
        m_scheme->setSchedule(schedule);
}

int DeviceControllerCPP::getStartMode()
{
    auto sm = _deviceAPI->getStartMode();
    TRACE << "start mode is: " << sm;

    return sm;
}

bool DeviceControllerCPP::getUpdateMode()
{
    if (m_system)
        return m_system->updateSequenceOnStart();

    qWarning() << "system is not initialized correctly!";
    return false;
}

SystemSetup *DeviceControllerCPP::systemSetup() const {
    return mSystemSetup;
}

void DeviceControllerCPP::setSystemSetup(SystemSetup *systemSetup) {
    if (mSystemSetup == systemSetup)
        return;

    mSystemSetup = systemSetup;

    // Set system setp
    m_scheme->setSystemSetup(mSystemSetup);

    emit systemSetupChanged();
}

void DeviceControllerCPP::setMainData(QVariantMap mainData)
{
    bool isOk;
    double tc = mainData.value("temperature").toDouble(&isOk);
    if (isOk){
        mRawTemperature = tc;

        double dt = deltaCorrection();
        TRACE_CHECK(qAbs(mDeltaTemperatureIntegrator) > 1E-3) << "Delta T correction: Tnow " << tc << ", Tdelta " << dt;
        if (qAbs(dt) < 10) {
            mainData.insert("temperature", tc - dt);
        } else {
            qWarning() << "dt is greater than 10! check for any error.";
        }
    }

    if (_mainData == mainData)
        return;

    _mainData = mainData;

    if (m_scheme)
        m_scheme->setMainData(getMainData());
}

void DeviceControllerCPP::startTestMode()
{
    // Update test mode in system
    if (m_system)
        m_system->setTestMode(true);
}

void DeviceControllerCPP::checkUpdateMode()
{
    // check if updated
    bool updateMode = getUpdateMode();
    if (updateMode) { // or intial mode, in this case disable fetching after one time fetching
        //            Run API to get settings from server (sync, getWirings, )
        TRACE << "getting settings from server";
        if (m_system)
            m_system->getUpdate();
    }
}

bool DeviceControllerCPP::checkSN()
{
    auto state = _deviceAPI->checkSN();
    TRACE << "checkSN : " << state;

    bool snMode = state != 2;
    emit snModeChanged(snMode);

    return snMode;
}

void DeviceControllerCPP::checkContractorInfo()
{
    auto info = m_system->getContractorInfo();

    Q_EMIT contractorInfoUpdated(info.value("brand").toString(), info.value("phone").toString(),
                                 info.value("logo").toString(), info.value("url").toString(),
                                 info.value("tech").toString());
}

void DeviceControllerCPP::pushSettingsToServer(const QVariantMap &settings)
{
    m_system->pushSettingsToServer(settings);
}

void DeviceControllerCPP::setOverrideMainData(QVariantMap mainDataOverride)
{
    if (_mainData_override == mainDataOverride)
        return;

    _mainData_override = mainDataOverride;

    if (m_scheme)
        m_scheme->setMainData(getMainData());
}

bool DeviceControllerCPP::setFan(AppSpecCPP::FanMode fanMode, int newFanWPH)
{
    if (m_scheme) {
        m_scheme->setFan(fanMode, newFanWPH);
        return true;
    }

    return false;
}

QVariantMap DeviceControllerCPP::getMainData()
{
    auto mainData = _mainData;

    auto overrideData = _mainData_override;
#ifdef __unix__
    QSettings override("/usr/local/bin/override.ini", QSettings::IniFormat);
    bool hasOverride = override.value("on").toBool();
    if (hasOverride) {
        auto overrideTemp = override.value("temp").toDouble();
        TRACE_CHECK(!_override_by_file) <<"temperature will be overriden by value: " << overrideTemp << ", read from /usr/local/bin/override.ini file.";
        overrideData.insert("temperature", overrideTemp);
    }

    if (_override_by_file != hasOverride){
        TRACE_CHECK(!hasOverride) << "temperature will not be overriden anymore.";
        _override_by_file = hasOverride;
    }
#endif

    for (const auto &pair : overrideData.toStdMap()) {
        TRACE << pair.first << "with value: " << mainData.value(pair.first) << ", replaced by: " << pair.second;
        mainData.insert(pair.first, pair.second);
    }

    // to make effect of overriding instantly, TODO  remove or disable later
    bool isOk;
    double currentTemp = mainData.value("temperature").toDouble(&isOk);
    if (isOk && currentTemp != _temperatureLast) {
        _temperatureLast = currentTemp;
        if (m_scheme)
            m_scheme->setMainData(mainData);
    }

    return mainData;
}
