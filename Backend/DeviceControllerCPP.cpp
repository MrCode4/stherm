#include "DeviceControllerCPP.h"

#include "LogCategoires.h"
#include "SchemeDataProvider.h"
#include "ScreenSaverManager.h"
#include "DeviceInfo.h"

/* ************************************************************************************************
 * Log properties
 * ************************************************************************************************/
#ifdef DEBUG_MODE
static  const QString m_DateTimeHeader        = "DateTime UTC (sec)";
static  const QString m_DeltaCorrectionHeader = "Delta Correction (F)";
static  const QString m_DTIHeader             = "Delta Temperature Integrator";
static  const QString m_BacklightFactorHeader = "backlightFactor";
static  const QString m_BrightnessHeader      = "Brightness (%)";
static  const QString m_RawTemperatureHeader  = "Raw Temperature (F)";
static  const QString m_NightModeHeader       = "Is Night Mode Running";
static  const QString m_BacklightRHeader      = "Backlight - R";
static  const QString m_BacklightGHeader      = "Backlight - G";
static  const QString m_BacklightBHeader      = "Backlight - B";
static  const QString m_LedEffectHeader       = "Backlight - LED effect";
static  const QString m_CPUUsage              = "CPU Usage (%)";
static  const QString m_FanStatus             = "Fan status";
static  const QString m_BacklightState        = "Backlight state";
static  const QString m_T1                    = "Temperature compensation T1 (F) - fan effect";
#endif
static  const QString m_RestartAfetrSNTestMode  = "RestartAfetrSNTestMode";

static  const char* m_GetOutdoorTemperatureReceived  = "GetOutdoorTemperatureRecieved";

static const QByteArray m_default_backdoor_backlight = R"({
    "red": 255,
    "green": 255,
    "blue": 255,
    "mode": 0,
    "on": // true
}
)";

static const QByteArray m_default_backdoor_brightness = R"({
    "brightness": // 100
}
)";

static const QByteArray m_default_backdoor_fan = R"({
    "speed": // 100
}
)";

static const QByteArray m_default_backdoor_relays = R"({
    "o_b": // 1,
    "y1": //1,
    "y2": // 1,
    "w1": //1,
    "w2": //1,
    "w3": //1,
    "acc2": //1,
    "acc1p": //1,
    "acc1n": //1,
    "g": //1
}
)";


//! Set CPU governer in the zeus base system
//! It is strongly dependent on the kernel.
inline void setCPUGovernorMode(QString governer) {
#ifdef __unix__
    QDir cpuDir("/sys/devices/system/cpu/");
    QStringList cpuList = cpuDir.entryList(QStringList() << "cpu[0-9]*");

    LOG_DC << "CPU List: =" << cpuList;

    foreach (const QString& cpu, cpuList) {
        QString governorFile = QString("/sys/devices/system/cpu/%1/cpufreq/scaling_governor").arg(cpu);
        QFile file(governorFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            QTextStream out(&file);
            out << governer; // Set CPU governor
            file.close();
            LOG_DC << "Set CPU" << cpu << "governor to " << governer;
        } else {
            LOG_DC << "Failed to set CPU" << cpu << "governor to performance";
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
    , mSchemeDataProvider(new SchemeDataProvider(this))
    , mTempScheme(new Scheme(_deviceAPI, mSchemeDataProvider, this))
    , mHumidityScheme(new HumidityScheme(_deviceAPI, mSchemeDataProvider, this))
    , mDeviceHasInternet(false)
    , mIsNeedOutdoorTemperature(false)
    , mIsEligibleOutdoorTemperature(false)
{

    m_system = _deviceAPI->system();
    m_sync = _deviceAPI->sync();

    connect(m_system, &NUVE::System::contractorInfoReady, this, [this]() {
        auto info = m_system->getContractorInfo();
        emit contractorInfoUpdated(info.value("brand").toString(), info.value("phone").toString(),
                                     info.value("logo").toString(), info.value("url").toString(),
                                     info.value("tech").toString());
    });

    // When system is OFF, the set temperature will remain constant
    connect(mSchemeDataProvider.get(), &SchemeDataProvider::effectiveTemperatureChanged, this, &DeviceControllerCPP::effectiveTemperatureChanged);

    mAdaptiveBrightness = 50;

    QDir backdoorDir(m_backdoorPath);
    if (!backdoorDir.exists())
        backdoorDir.mkpath(m_backdoorPath);

    for (const QString& fileName : m_watchFiles)
    {
        QString path = m_backdoorPath + fileName;
        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(defaultSettings(path));
            file.close();
            qInfo() << "Backdoor setting file" << path << "reset to default! finalize it to apply.";
        } else {
            qCritical() << "Backdoor setting file" << path << "can not be opened to be reset";
        }

        m_fileSystemWatcher.addPath(path);
    }
    connect(&m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &DeviceControllerCPP::processBackdoorSettingFile);

    // Update backlight
    mBacklightTimer.setTimerType(Qt::PreciseTimer);
    mBacklightTimer.setSingleShot(true);
    connect(&mBacklightTimer, &QTimer::timeout, this, [this]() {
        auto colorData = mBacklightTimer.property("color").value<QVariantList>();
        LOG_DC << "restoring color with timer " << colorData;
        setBacklight(colorData, true);
    });

    connect(mTempScheme, &Scheme::started, this, [this]() {
        emit tempSchemeStateChanged(true);
    });
    connect(mTempScheme, &Scheme::finished, this, [this]() {
        emit tempSchemeStateChanged(false);
    });
    connect(mHumidityScheme, &Scheme::started, this, [this]() {
        emit humiditySchemeStateChanged(true);
    });
    connect(mHumidityScheme, &Scheme::finished, this, [this]() {
        emit humiditySchemeStateChanged(false);
    });

    connect(mTempScheme, &Scheme::alert, this, [this]() {
        emit alert(STHERM::AlertLevel::LVL_Emergency,
                   AppSpecCPP::AlertTypes::Alert_Efficiency_Issue,
                   STHERM::getAlertTypeString(AppSpecCPP::Alert_Efficiency_Issue));
    });

    // TODO should be loaded later for accounting previous session
    mDeltaTemperatureIntegrator = 0;

    // Default value
    mFanSpeed = 16;
    mFanOff = false;

    mNightModeTimer.setTimerType(Qt::PreciseTimer);
    mNightModeTimer.setInterval(5000 * 60);
    mNightModeTimer.setSingleShot(true);
    connect(&mNightModeTimer, &QTimer::timeout, this, [this]() {
        setFanSpeed(0);
    });

    connect(_deviceIO, &DeviceIOController::fanStatusUpdated, this, [this](bool fanOff) {
        mFanOff = fanOff;
    });

    connect(_deviceIO, &DeviceIOController::forceOffSystem, this, [this](bool forceOff) {

        if (forceOff) {
            emit forceOffSystem();

        } else if (mSystemSetup->_mIsSystemShutoff) {
            emit exitForceOffSystem();
        }
    });

    connect(_deviceIO, &DeviceIOController::co2SensorStatus, this, &DeviceControllerCPP::co2SensorStatus);
    connect(_deviceIO, &DeviceIOController::temperatureSensorStatus, this, &DeviceControllerCPP::temperatureSensorStatus);
    connect(_deviceIO, &DeviceIOController::humiditySensorStatus,    this, &DeviceControllerCPP::humiditySensorStatus);

    mTEMPERATURE_COMPENSATION_Timer.setTimerType(Qt::PreciseTimer);
    mTEMPERATURE_COMPENSATION_Timer.setInterval(1000);
    mTEMPERATURE_COMPENSATION_Timer.setSingleShot(false);
    connect(&mTEMPERATURE_COMPENSATION_Timer, &QTimer::timeout, this, [this]() {
        if (isFanON()) {
            mTEMPERATURE_COMPENSATION_T1 = mTEMPERATURE_COMPENSATION_T1 + (0.2 / 1.8 - mTEMPERATURE_COMPENSATION_T1) / 148.4788;
        } else {
            mTEMPERATURE_COMPENSATION_T1 = mTEMPERATURE_COMPENSATION_T1 + ((2.847697 - deltaCorrection()) - mTEMPERATURE_COMPENSATION_T1) / 655.5680515;
        }

#ifdef DEBUG_MODE
        DC_LOG << "Temperature Correction - T1: "<< mTEMPERATURE_COMPENSATION_T1 << "- Fan running: " << isFanON();
#endif
    });
    mTEMPERATURE_COMPENSATION_Timer.start();

    // Thge system prepare the direcories for usage
    m_system->mountDirectory("/mnt/data", "/mnt/data/sensor");
    mGeneralSystemDatafilePath = QString("/mnt/data/sensor/gsd-%0.csv").arg(QDateTime::currentSecsSinceEpoch());

    mIsNightModeRunning = false;

#ifdef DEBUG_MODE
    mLogTimer.setTimerType(Qt::PreciseTimer);
    mLogTimer.start(1000);
    connect(&mLogTimer, &QTimer::timeout, this, [this]() {
        DC_LOG << "---------------------- Start General System Data Log ----------------------";

        auto cpuData = m_system->cpuInformation();
        auto brightness = UtilityHelper::brightness();

        DC_LOG << "Delta Correction: " << deltaCorrection() <<
            "- Delta Temperature Integrator: " << mDeltaTemperatureIntegrator <<
            "- backlightFactor: " << _deviceIO->backlightFactor();


        DC_LOG << "Brightness: " << brightness;

        DC_LOG << "Raw Temperature: " << mRawTemperature << _mainData.value(temperatureKey);
        DC_LOG << "Corrected Temperature: " << mRawTemperature << _mainData.value(temperatureUIKey);

        DC_LOG << "Is night mode running: " << mIsNightModeRunning;

        writeGeneralSysData(cpuData, brightness);

        DC_LOG << "---------------------- End General System Data Log ----------------------";
    });

#endif


    mBacklightPowerTimer.setTimerType(Qt::PreciseTimer);
    mBacklightPowerTimer.setSingleShot(false);
    mBacklightPowerTimer.setInterval(1000);
    connect(&mBacklightPowerTimer, &QTimer::timeout, this, [this]() {
        mDeltaTemperatureIntegrator *= TEMPERATURE_INTEGRATOR_DECAY_CONSTANT;
        mDeltaTemperatureIntegrator += _deviceIO->backlightFactor();
    });
    mBacklightPowerTimer.start();

    connect(_deviceIO, &DeviceIOController::mainDataReady, this, [this](QVariantMap data) {
        // To avoid the first deviation in the average
        if (!_isFirstDataReceived) {
            _rawMainData = data;
            _isFirstDataReceived = true;
        }

        setMainData(data);
    });

    // Update nrf version
    connect(_deviceIO, &DeviceIOController::nrfVersionUpdated, this, [this]() {
        emit nrfVersionChanged();

        LOG_DC << getNRF_SW();
        if (getNRF_SW() != "01.10-RC1") {
            LOG_DC << "start firmware update in 3 seconds";
            QTimer::singleShot(3000, this, [this]() {updateNRFFirmware();});
        } else if (!mBacklightModelData.empty()){
            setBacklight(mBacklightModelData, true);
        }
    });

    // Update ti version
    connect(_deviceIO, &DeviceIOController::tiVersionUpdated, this, [this]() {
        emit tiVersionChanged();
    });

    connect(_deviceIO, &DeviceIOController::tofDataReady, this, [this](QVariantMap data) {
        setMainData(data, true);
    });

    connect(_deviceIO, &DeviceIOController::adaptiveBrightness, this, [this](double adaptiveBrightness) {
        setAdaptiveBrightness(adaptiveBrightness);
    });

    connect(_deviceIO, &DeviceIOController::alert, this, [this](STHERM::AlertLevel alertLevel,
                                                                AppSpecCPP::AlertTypes alertType,
                                                                QString alertMessage) {
        emit alert(alertLevel,
                   alertType,
                   alertMessage);
    });

    mGetOutdoorTemperatureTimer.setInterval(60 * 1000);
    mGetOutdoorTemperatureTimer.setSingleShot(false);
    // To get the outdoor temperature for first time.
    mGetOutdoorTemperatureTimer.setProperty(m_GetOutdoorTemperatureReceived, false);
    connect(&mGetOutdoorTemperatureTimer, &QTimer::timeout, this, [this]() {
        m_sync->getOutdoorTemperature();
    });

    // Get the outdoor temperature when serial number is ready.
    connect(m_sync, &NUVE::Sync::serialNumberReady, this, [this]() {
        checkForOutdoorTemperature();
    });

    connect(m_sync, &NUVE::Sync::outdoorTemperatureReady, this, [this](bool success, double temp) {
        LOG_DC << "Outdoor temperature:" << success << temp;
        if (success) {
            mSchemeDataProvider->setOutdoorTemperature(temp);

            mGetOutdoorTemperatureTimer.setProperty(m_GetOutdoorTemperatureReceived, true);
            checkForOutdoorTemperature();
        }
    });

    connect(mTempScheme, &Scheme::changeBacklight, this, [this](QVariantList color, QVariantList afterColor) {

        if (mBacklightTimer.isActive())
            mBacklightTimer.stop();

        if (mIsNightModeRunning) {
            return;
        }

        LOG_CHECK_DC(false) << "Update backlight." << color << afterColor << mBacklightModelData;

        if (color.isEmpty()) {
            LOG_DC << "restoring color with force " << mBacklightModelData;
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

    connect(mTempScheme, &Scheme::updateRelays, this, [this](STHERM::RelayConfigs relays, bool force) {
        if (mBackdoorSchemeEnabled)
            return;

        _deviceIO->updateRelays(relays, force);
    });

    connect(mTempScheme, &Scheme::startSystemDelayCountdown, this, &DeviceControllerCPP::startSystemDelayCountdown);
    connect(mTempScheme, &Scheme::stopSystemDelayCountdown, this, &DeviceControllerCPP::stopSystemDelayCountdown);
    connect(mTempScheme, &Scheme::currentSystemModeChanged, this, &DeviceControllerCPP::currentSystemModeChanged);
    connect(mTempScheme, &Scheme::actualModeStarted, this, &DeviceControllerCPP::actualModeStarted);
    connect(mTempScheme, &Scheme::dfhSystemTypeChanged, this, &DeviceControllerCPP::dfhSystemTypeChanged);
    connect(mTempScheme, &Scheme::manualEmergencyModeUnblockedAfter, this, &DeviceControllerCPP::manualEmergencyModeUnblockedAfter);
    connect(mTempScheme, &Scheme::sendRelayIsRunning, this, [this] (const bool& isRunning) {
        if (mBackdoorSchemeEnabled)
            return;

        mHumidityScheme->setCanSendRelays(!isRunning);
        }, Qt::DirectConnection);

    connect(mHumidityScheme, &HumidityScheme::sendRelayIsRunning, this, [this] (const bool& isRunning) {
        if (mBackdoorSchemeEnabled)
            return;

        mTempScheme->setCanSendRelays(!isRunning);
    }, Qt::DirectConnection);

    connect(mHumidityScheme, &HumidityScheme::updateRelays, this, [this](STHERM::RelayConfigs relays, bool force) {
        if (mBackdoorSchemeEnabled)
            return;

        _deviceIO->updateRelays(relays, force);
    });

    if (m_system) {
        connect(m_system, &NUVE::System::systemUpdating, this, [this]() {
            mTempScheme->moveToUpdatingMode();
        });

        // Thge system prepare the direcories for usage
        m_system->mountDirectory("/mnt/data", "/mnt/data/sensor");
    }

    connect(_deviceIO, &DeviceIOController::relaysUpdated, this, [this](STHERM::RelayConfigs relays) {
        emit fanWorkChanged(relays.g == STHERM::ON);
    });

    // save data to csv file
    // update the timer intervals when sample rate changed.
    connect(&mSaveSensorDataTimer, &QTimer::timeout, this, [this]() {
        auto elapsed = mResponsivenessTimer.restart();
        LOG_CHECK_DC(false) << "Operations took by :" << elapsed;
        if (elapsed - mSaveSensorDataTimer.interval() > 3000) {
            LOG_DC << "Operations delayed by :" << elapsed << mSaveSensorDataTimer.interval();
        }
        writeSensorData(_mainData);
    });

    auto sampleRate = _deviceAPI->deviceConfig().sampleRate;
    mSaveSensorDataTimer.setTimerType(Qt::PreciseTimer);
    // TODO: check start condition
    mSaveSensorDataTimer.start(sampleRate * 60 * 1000);
    mResponsivenessTimer.start();

    //! Set sInstance to this
    if (!sInstance) {
        sInstance = this;
    }
}

DeviceControllerCPP::~DeviceControllerCPP() {}

void DeviceControllerCPP::setSampleRate(const int sampleRate) {
    _deviceAPI->setSampleRate(sampleRate);

    if (mSaveSensorDataTimer.isActive()) {
        mSaveSensorDataTimer.stop();

        auto sr = _deviceAPI->deviceConfig().sampleRate;
        // TODO: check start condition
        mSaveSensorDataTimer.start(sr * 60 * 1000);
        mResponsivenessTimer.restart();
    }
}

bool DeviceControllerCPP::setBacklight(QVariantList data, bool isScheme)
{
    bool success = _deviceIO->setBacklight(data);

    if (success && !isScheme) {
        mBacklightModelData = data;
    }
    mBacklightActualData = data;

    return success;
}

//! TODO
//! Handle the CPU frequency or governor will be set to minimum speed level.
//! Handle other power limiting functions
void DeviceControllerCPP::nightModeControl(bool start)
{
    if (mIsNightModeRunning == start)
        return;

    mIsNightModeRunning = start;

    m_system->setNightModeRunning(start);

    if (start) {
        mNightModeTimer.start();

    } else {
        mNightModeTimer.stop();
        setFanSpeed(16); // 100 / 7
    }
}

void DeviceControllerCPP::setCPUGovernor(AppSpecCPP::CPUGovernerOption CPUGovernerOption)
{
    if (CPUGovernerOption == mCPUGoverner)
        return;

    QString governer;
    switch (CPUGovernerOption) {
    case AppSpecCPP::CPUGpowersave:
        governer = "powersave";
        break;

    case AppSpecCPP::CPUGondemand:
        governer = "ondemand";
        break;

    case AppSpecCPP::CPUGperformance:
        governer = "performance";
        break;

    default:
        break;
    }

    if (!governer.isEmpty()){
        mCPUGoverner = CPUGovernerOption;
        setCPUGovernorMode(governer);
    }
}

void DeviceControllerCPP::forgetDevice()
{
    QFile::remove("/usr/local/bin/QSCore.cfg");
    _deviceAPI->forgetDevice();
    m_system->forgetDevice();
}

double DeviceControllerCPP::adaptiveBrightness() {
    return mAdaptiveBrightness;
}

bool DeviceControllerCPP::setSettings(QVariantList data)
{
    if (_deviceIO->setSettings(data)){
        mSettingsModelData = data;
        return true;
    }

    return false;
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

    // Update vacations
    if (!mSchemeDataProvider.isNull())
        mSchemeDataProvider->setVacation(vacation);
}

void DeviceControllerCPP::setRequestedTemperature(const double temperature)
{
    if (!mSchemeDataProvider.isNull())
        mSchemeDataProvider->setSetPointTemperature(temperature);
}

void DeviceControllerCPP::setRequestedHumidity(const double humidity)
{
    if (!mSchemeDataProvider.isNull())
        mSchemeDataProvider->setRequestedHumidity(humidity);
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
    int startMode;
#ifdef INITIAL_SETUP_MODE_ON
    startMode = 1;
#elif defined(TROUBLESHOOTING_MODE_ON)
    startMode = 0;
#else
    startMode = getStartMode();
#endif

    emit startModeChanged(startMode);

    mIsDeviceStarted = true;

    if (startMode == 0) {
        startTestMode();

        // if test mode returns, when the other codes should be run? after finishing test mode? //TODO
        return;
    }

    if (!checkSN()){
        LOG_DC << "INITAIL SETUP";
        return;
    }

    // checkUpdateMode();
}

void DeviceControllerCPP::stopDevice()
{
    _deviceIO->stopReading();
    runTemperatureScheme(false);
    runHumidityScheme(false);
}

void DeviceControllerCPP::runTemperatureScheme(bool start)
{
    LOG_DC << "starting temperature scheme: " << (start && mIsDeviceStarted) << start;
    if(start && mIsDeviceStarted) {
        // Start with delay to ensure the model loaded.
        // will be loaded always, but should be OFF in initial setup mode as its default is OFF !!!
        // This function will execute after provided sensor data has been received,
        // so we do not need more delay.
        if (!mTempScheme->isRunning())
            mTempScheme->restartWork(true);

    } else {
        mTempScheme->stop();
    }
}

void DeviceControllerCPP::runHumidityScheme(bool start)
{
    LOG_DC << "starting humidity scheme: " << (start && mIsDeviceStarted) << start;
    if(start && mIsDeviceStarted) {
        // Start with delay to ensure the model loaded.
        // will be loaded always, but should be OFF in initial setup mode as its default is OFF !!!
        // This function will execute after provided sensor data has been received,
        // so we do not need more delay.
        if (!mHumidityScheme->isRunning())
            mHumidityScheme->restartWork(true);

    } else {
        mHumidityScheme->stop();
    }
}

void DeviceControllerCPP::setActivatedSchedule(ScheduleCPP *schedule)
{
    if (schedule && mSchemeDataProvider->effectiveSystemMode() == AppSpecCPP::Off) {
        LOG_DC << "An schedule with name " << schedule->name << "is active while mode is off";
    } else if (schedule) {
        LOG_CHECK_DC(false) << "An schedule with name " << schedule->name
                           << "is active while mode is off";
    } else {
        LOG_CHECK_DC(false)
            << "The schedule is inactive, hence the system reverts to its previous mode ("
            << mSchemeDataProvider->effectiveSystemMode() << ").";
    }

    if (!mSchemeDataProvider.isNull())
        mSchemeDataProvider->setSchedule(schedule);
}

int DeviceControllerCPP::getStartMode()
{
    auto sm = _deviceAPI->getStartMode();
    LOG_DC << "start mode is: " << sm;

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
    mSchemeDataProvider->setSystemSetup(mSystemSetup);

    // To provide outdoor temperature
    connect(mSystemSetup, &SystemSetup::systemTypeChanged, this, [=] {
        checkForOutdoorTemperature();
    });

    // To provide outdoor temperature
    connect(mSystemSetup, &SystemSetup::isAUXAutoChanged, this, [=] {
        checkForOutdoorTemperature();
    });

    checkForOutdoorTemperature();

    emit systemSetupChanged();
}

void DeviceControllerCPP::checkForOutdoorTemperature() {
    updateIsEligibleOutdoorTemperature();
    updateIsNeedOutdoorTemperature();

    if (mIsEligibleOutdoorTemperature && mIsNeedOutdoorTemperature) {

        if (!mGetOutdoorTemperatureTimer.isActive()) {
            m_sync->getOutdoorTemperature();
            mGetOutdoorTemperatureTimer.start();
        }

        // To cache the outdoor temperature, it will be stop when get data successfully in the other system types
    } else if (mIsEligibleOutdoorTemperature && !mGetOutdoorTemperatureTimer.property(m_GetOutdoorTemperatureReceived).toBool()) {

        m_sync->getOutdoorTemperature();
        if (!mGetOutdoorTemperatureTimer.isActive())
            mGetOutdoorTemperatureTimer.start();

    } else if (mGetOutdoorTemperatureTimer.isActive()) {
        LOG_DC << "Stop to get the outdoor temperature " << mDeviceHasInternet << mSystemSetup->systemType << m_sync->getSerialNumber();
        mGetOutdoorTemperatureTimer.stop();
    }
}

void DeviceControllerCPP::setMainData(QVariantMap mainData, bool addToData)
{
    if (addToData) {
        //! Insert data to main data.
        for (const auto &pair : mainData.toStdMap()) {
            _mainData.insert(pair.first, pair.second);
        }

    } else {
        bool isOk;
        double tc = mainData.value(temperatureKey).toDouble(&isOk);
        if (isOk){
            mRawTemperature = tc;

            double dt = deltaCorrection();
            // Fan status effect:
            dt += mTEMPERATURE_COMPENSATION_T1;
            LOG_CHECK_DC(qAbs(mDeltaTemperatureIntegrator) > 1E-3) << "Delta T correction: Tnow " << tc << ", Tdelta " << dt;
            if (qAbs(dt) < 10) {
                tc -= dt;

                mainData.insert(temperatureUIKey, tc);
            } else {
                qWarning() << "dt is greater than 10! check for any error.";
            }
        }

        if (mFanOff)
            mainData.insert(fanSpeedKey, 0);

        if (_mainData == mainData)
            return;

        _mainData = mainData;

        // Average of the last two temperature and humidity
        // Get temperature from _rawMainData
        auto rt = _rawMainData.value(temperatureUIKey, tc).toDouble(&isOk);

        if (isOk)
            _mainData.insert(temperatureUIKey, (tc + rt) / 2);

        auto mh = mainData.value(humidityKey, 0.0).toDouble();
        // Get humidity from _rawMainData
        auto rh = _rawMainData.value(humidityKey, mh).toDouble(&isOk);

        if (isOk)
            _mainData.insert(humidityKey, (mh + rh) / 2);

        _rawMainData = mainData;
    }

    if (!mSchemeDataProvider.isNull())
        mSchemeDataProvider->setMainData(getMainData());
}

void DeviceControllerCPP::startTestMode()
{
    // Update test mode in system
    if (m_system)
        m_system->setTestMode(true);
}

bool DeviceControllerCPP::checkUpdateMode()
{
    // check if updated
    bool updateMode = getUpdateMode();
    if (updateMode) { // or intial mode, in this case disable fetching after one time fetching
        //            Run API to get settings from server (sync, getWirings, )
        LOG_DC << "getting settings from server";
        //if (m_system)
        //    m_system->getUpdate();
    }

    return updateMode;
}

void DeviceControllerCPP::wifiConnected(bool hasInternet)
{
    m_system->wifiConnected(hasInternet);

    mDeviceHasInternet = hasInternet;

    checkForOutdoorTemperature();
}

void DeviceControllerCPP::lockDeviceController(bool isLock)
{
    // TODO
    return;

    if (isLock) {
        _deviceIO->stopTOFGpioHandler();

    } else {
        _deviceIO->startTOFGpioHandler();
    }
}

void DeviceControllerCPP::setAdaptiveBrightness(const double adaptiveBrightness) {
    if (mAdaptiveBrightness == adaptiveBrightness)
        return;

    mAdaptiveBrightness = adaptiveBrightness;
    emit adaptiveBrightnessChanged();
}

bool DeviceControllerCPP::isFanON()
{
    return !mFanOff;
}

void DeviceControllerCPP::processBackdoorSettingFile(const QString &path)
{
    if (path.endsWith("backlight.json")){
        processBackLightSettings(path);
    } else if (path.endsWith("fan.json")) {
        processFanSettings(path);
    } else if (path.endsWith("brightness.json")) {
        processBrightnessSettings(path);
    } else if (path.endsWith("relays.json")) {
        processRelaySettings(path);
    } else {
        qWarning() << "Incompatible backdoor file, processed nothing";
    }
}

bool DeviceControllerCPP::checkSN()
{
    auto state = _deviceAPI->checkSN();
    LOG_DC << "checkSN : " << state;

    bool snMode = state != 2;

    // Active screen saver
    if (snMode)
        ScreenSaverManager::instance()->setAppActive(true);

    // System is no need update in snMode === 0
    // After forget device state can not be zero (0)
    if (state == 0)
        m_system->setIsInitialSetup(false);

    emit snModeChanged(state);

    return snMode;
}

void DeviceControllerCPP::checkContractorInfo()
{
    m_system->fetchContractorInfo();
}

void DeviceControllerCPP::pushSettingsToServer(const QVariantMap &settings)
{
    m_system->pushSettingsToServer(settings);
}

void DeviceControllerCPP::setEndpoint(const QString &subdomain, const QString &domain)
{
    _deviceAPI->setEndpoint(
        QString("https://%0%1/").arg(subdomain.isEmpty() ? "" : subdomain + ".", domain));
}

QString DeviceControllerCPP::getEndpoint()
{
    auto endpoint = QString::fromStdString(_deviceAPI->deviceConfig().endpoint);

    // Use QUrl to parse the URL
    QUrl parsedUrl(endpoint);

    // Get the host (domain + subdomain) as QString
    QString host = parsedUrl.host();

    return host;
}

void DeviceControllerCPP::pushAutoSettingsToServer(const double& auto_temp_low, const double& auto_temp_high)
{
    m_system->pushAutoSettingsToServer(auto_temp_low, auto_temp_high);
}

void DeviceControllerCPP::setOverrideMainData(QVariantMap mainDataOverride)
{
    if (_mainData_override == mainDataOverride)
        return;

    _mainData_override = mainDataOverride;

    if (!mSchemeDataProvider.isNull())
        mSchemeDataProvider->setMainData(getMainData());
}

bool DeviceControllerCPP::setFan(AppSpecCPP::FanMode fanMode, int newFanWPH)
{
    if (mTempScheme) {
        mTempScheme->setFan(fanMode, newFanWPH);
        return true;
    }

    return false;
}

void DeviceControllerCPP::switchDFHActiveSysType(AppSpecCPP::SystemType activeSystemType) {
    mTempScheme->switchDFHActiveSysType(activeSystemType);
}

bool DeviceControllerCPP::isTestsPassed()
{
    QStringList failedTests;
    for (const auto &testName : mAllTestNames) {
        auto resultIter = mAllTestsResults.find(testName);
        // whether not found in results or the value is false
        if (resultIter == mAllTestsResults.end() || !resultIter->second)
            failedTests.append(testName);
    }

    return failedTests.empty();
}

void DeviceControllerCPP::setAutoMinReqTemp(const double cel_value)
{
    if (mSchemeDataProvider.isNull()) {
        LOG_DC << "The schedule data provider is not available.";
        return;
    }

    mSchemeDataProvider->setAutoMinReqTemp(cel_value);
}

void DeviceControllerCPP::setAutoMaxReqTemp(const double cel_value)
{
    if (mSchemeDataProvider.isNull()) {
        LOG_DC << "The schedule data provider is not available.";
        return;
    }

    mSchemeDataProvider->setAutoMaxReqTemp(cel_value);
}

bool DeviceControllerCPP::updateNRFFirmware()
{

    LOG_DC << "NRF Hardware: " << getNRF_HW() <<
        "NRF software:" << getNRF_SW();
    if (m_system->installUpdate_NRF_FW_Service()){
        emit nrfUpdateStarted();
        return _deviceIO->update_nRF_Firmware();
    }
    return false;
}

bool DeviceControllerCPP::checkNRFFirmwareVersion()
{
#ifdef __unix__
    auto nrfSW = getNRF_SW();
    auto appVersion = qApp->applicationVersion().split(".");

    if (appVersion.length() < 3) {
        qWarning() << "The app version is wrong." << appVersion;
        return false;
    }

    if (nrfSW.length() < 3) {
        qWarning() << "The nrf version is wrong." << nrfSW;
        return false;
    }

    LOG_DC << "NRF  Version: " << nrfSW << " - Application version: " << appVersion;

    bool firmwareUpdated = nrfSW == "01.10-RC1";
    bool appHasFirmwreUpdate = appVersion.at(0).toInt() > 0 ||
                               (appVersion.at(0).toInt() == 0 && appVersion.at(1).toInt() > 3) ||
                               (appVersion.at(0).toInt() == 0 && appVersion.at(1).toInt() == 3 && appVersion.at(2).toInt() >= 6);

    // The app version should be higher than 0.3.6 if the nRF SW version is 01.10RC1
    // nRF SW version is not 01.10RC1, app Should not be greater than 0.3.5
    return  firmwareUpdated == appHasFirmwreUpdate;
#endif
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
        LOG_CHECK_DC(!_override_by_file) <<"temperature will be overriden by value: " << overrideTemp << ", read from /usr/local/bin/override.ini file.";
        overrideData.insert("temperature", overrideTemp);
    }

    if (_override_by_file != hasOverride){
        LOG_CHECK_DC(!hasOverride) << "temperature will not be overriden anymore.";
        _override_by_file = hasOverride;
    }
#endif

    for (const auto &pair : overrideData.toStdMap()) {
        LOG_DC << pair.first << "with value: " << mainData.value(pair.first) << ", replaced by: " << pair.second;
        mainData.insert(pair.first, pair.second);
    }

    // to make effect of overriding instantly, TODO  remove or disable later
    bool isOk;
    double currentTemp = mainData.value("temperature").toDouble(&isOk);
    if (isOk && currentTemp != _temperatureLast) {
        _temperatureLast = currentTemp;
        if (mSchemeDataProvider.isNull())
            mSchemeDataProvider->setMainData(mainData);
    }

    return mainData;
}

double DeviceControllerCPP::getTemperature()
{
    return mSchemeDataProvider->currentTemperature();
}

void DeviceControllerCPP::writeTestResult(const QString &fileName, const QString &testName, const QString& testResult, const QString &description)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << "Unable to open file" << file.fileName() << "for writing";
        return;
    }

    QTextStream output(&file);
    output << testName << "," << testResult << "," << description << "\n";
}

void DeviceControllerCPP::saveTestResult(const QString &testName, bool testResult, const QString &description)
{
    mAllTestsResults.insert_or_assign(testName, testResult);
    mAllTestsValues.insert_or_assign(testName, description);

    // keep the order on first occurance
    if (!mAllTestNames.contains(testName))
        mAllTestNames.push_back(testName);

    QString result = testResult ? "PASS" : "FAIL";
    writeTestResult("/test_results.csv", testName, result, description);
}

QString DeviceControllerCPP::beginTesting()
{
    mAllTestsValues.clear();
    mAllTestsResults.clear();
    mAllTestNames.clear();
    //! TODO initialize all tests in mAllTestNames

    QFile file("/test_results.csv");
    if (file.exists() && !file.remove())
    {
        qWarning() << "Unable to delete file" << file.fileName();
    }
    writeTestResult("/test_results.csv", "Test name", QString("Test Result"), "Description");

    QString uid = _deviceAPI->uid();
    QString sn = m_system->serialNumber();
    QString sw = QCoreApplication::applicationVersion();
    QString qt = qVersion();
    QString nrf = getNRF_SW();
    QString kernel = m_system->kernelBuildVersion();
    QString ti = getTI_SW();

    QString err;

    if (uid.isEmpty())
        err = "uid is empty.";
    saveTestResult("UID", !uid.isEmpty(), uid);

    // we do not throw error here, as it may retrieved at the end of process
    saveTestResult("SN", !sn.isEmpty(), sn);

    if (sw.isEmpty())
        err = "Software version is empty.";
    saveTestResult("SW version", !sw.isEmpty(), sw);

    if (qt.isEmpty())
        err = "Software version is empty.";
    saveTestResult("QT version", !qt.isEmpty(), qt);

    if (nrf.isEmpty())
        err = "NRF version is empty.";
    saveTestResult("NRF version", !nrf.isEmpty(), nrf);

    if (kernel.isEmpty())
        err = "Kernel is empty.";
    saveTestResult("Kernel version", !kernel.isEmpty(), kernel);


    if (ti.isEmpty())
        err = "TI software version is empty.";
    saveTestResult("TI version", !ti.isEmpty(), ti);

    return err;
}

void DeviceControllerCPP::testBrightness(int value)
{
    _deviceIO->setBrightnessTest(value);
}

void DeviceControllerCPP::stopTestBrightness()
{
    LOG_DC;
    _deviceIO->setBrightnessTest(0, false);
}

void DeviceControllerCPP::testFinished()
{
    // Override sn test on finished.
    QString sn = m_system->serialNumber();
    saveTestResult("SN", !sn.isEmpty(), sn);

    LOG_DC << "test finsihed with SN: " << sn;


    QStringList failedTests;
    for (const auto &testName : mAllTestNames) {
        auto resultIter = mAllTestsResults.find(testName);
        // whether not found in results or the value is false
        if (resultIter == mAllTestsResults.end() || !resultIter->second)
            failedTests.append(testName);
    }

    LOG_CHECK_DC(!failedTests.empty()) << "Failed tests are:" << failedTests;

    QString result = failedTests.empty() ? "PASS" : "FAIL";
    QString testResultsFileName = QString("/%1_%2.csv").arg(_deviceAPI->uid(), result);

    // Remove the file if exists
    if (QFileInfo::exists(testResultsFileName)) {
        if (!QFile::remove(testResultsFileName)) {
            LOG_DC << "Could not remove the file: " << testResultsFileName;
        }
    }

    // write header of the actual file
    writeTestResult(testResultsFileName, "Test name", QString("Test Result"), "Description");
    // write results to final file
    for (const auto &testName : mAllTestNames) {
        auto testResult = !failedTests.contains(testName);
        QString result = testResult ? "PASS" : "FAIL";
        QString description;
        auto descriptionIter = mAllTestsValues.find(testName);
        if (descriptionIter != mAllTestsValues.end())
                description = descriptionIter->second;
        writeTestResult(testResultsFileName, testName, result, description);
    }

    // Publish test results
    publishTestResults(testResultsFileName);

    // disabled it for now!
    if (false) {
        QSettings settings;
        settings.setValue(m_RestartAfetrSNTestMode, true);
    }

    LOG_DC << "testFinished";
}

bool DeviceControllerCPP::getSNTestMode() {
    QSettings settings;
    auto snTestMode = settings.value(m_RestartAfetrSNTestMode, false).toBool();
    settings.setValue(m_RestartAfetrSNTestMode, false);

    LOG_DC << "snTestMode" << snTestMode;
    return snTestMode;
}

void DeviceControllerCPP::writeGeneralSysData(const QStringList& cpuData, const int& brightness)
{
#ifdef DEBUG_MODE

    QStringList header = {m_DateTimeHeader, m_DeltaCorrectionHeader, m_T1, m_DTIHeader,
                          m_BacklightFactorHeader, m_BrightnessHeader, m_RawTemperatureHeader,
                          m_NightModeHeader, m_BacklightState, m_BacklightRHeader, m_BacklightGHeader,
                          m_BacklightBHeader, m_LedEffectHeader, m_CPUUsage, m_FanStatus};


    QFile file(mGeneralSystemDatafilePath);

    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream out(&file);

        // Check the header
        auto checkHeader = out.readAll().contains(m_DateTimeHeader);
        if (!checkHeader) {

            for (auto var = 0; var < cpuData.length(); var++) {
                header.append(QString("Temperature CPU%0").arg(var));
            }

            // Write header
            QStringList headerData;
            foreach (auto field, header) {
                headerData.append(field);
            }
            out << (headerData.join(",") + "\n");
        }

        // Write data rows
        QStringList dataStrList;
        auto backLightData = mBacklightActualData;
        if (mBacklightTimer.isActive()) {
            auto color = mBacklightTimer.property("color").value<QVariantList>();
            if (!color.isEmpty()) {
                backLightData = color;
            }
        }

        // Check backlight data.
        if (backLightData.size() != 5) {
            backLightData = QVariantList{-255, -255, -255, -1, "invalid"};
        }

        foreach (auto key, header) {
            if (key == m_DateTimeHeader) {
                dataStrList.append(QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()));

            } else if (key == m_DeltaCorrectionHeader) {
                dataStrList.append(QString::number(deltaCorrection() * 1.8));

            } else if (key == m_DTIHeader) {
                dataStrList.append(QString::number(mDeltaTemperatureIntegrator));

            } else if (key == m_BacklightFactorHeader) {
                dataStrList.append(QString::number(_deviceIO->backlightFactor()));

            } else if (key == m_BrightnessHeader) {
                dataStrList.append(QString::number(brightness));

            } else if (key == m_RawTemperatureHeader) {
                dataStrList.append(QString::number(mRawTemperature * 1.8 + 32));

            } else if (key == m_NightModeHeader) {
                dataStrList.append(mIsNightModeRunning ? "true" : "false");

            } else if (key == m_BacklightState) {
                dataStrList.append(backLightData[4].toString());

            }  else if (key == m_BacklightRHeader) {
                dataStrList.append(QString::number(backLightData[0].toInt() / 255.0));

            }  else if (key == m_BacklightGHeader) {
                dataStrList.append(QString::number(backLightData[1].toInt() / 255.0));

            }  else if (key == m_BacklightBHeader) {
                dataStrList.append(QString::number(backLightData[2].toInt() / 255.0));

            } else if (key == m_LedEffectHeader) {
                auto ledEffectInt = backLightData[3].toInt();
                QString ledEffect;
                switch (ledEffectInt) {
                case STHERM::LED_STABLE:
                    ledEffect = "Stable";
                    break;

                case STHERM::LED_FADE:
                    ledEffect = "FADE";
                    break;

                case STHERM::LED_BLINK:
                    ledEffect = "BLINK";
                    break;

                default:
                    ledEffect = "No Mode";
                    break;
                }

                dataStrList.append(ledEffect);

            } else if (key == m_CPUUsage) {
                dataStrList.append(QString::number(UtilityHelper::CPUUsage()));

            } else if (key == m_FanStatus) {
                dataStrList.append(isFanON() ? "On" : "Off");

            } else if (key == m_T1) {
                dataStrList.append(QString::number(mTEMPERATURE_COMPENSATION_T1 * 1.8));
            }
        }

        dataStrList.append(cpuData);
        out << (dataStrList.join(",")) << "\n";

        file.close();
        DC_LOG << "General System Data (csv) file written successfully in " << mGeneralSystemDatafilePath;

    } else {
        DC_LOG << "General System Data (csv) Failed to open the file for writing/Reading.";
    }
#endif
}

void DeviceControllerCPP::setFanSpeed(int speed)
{
    _deviceIO->setFanSpeed(speed);

    mFanSpeed = speed;
}

QJsonObject DeviceControllerCPP::processJsonFile(const QString &path, const QStringList &requiredKeys)
{
    if (!QFileInfo::exists(path))
    {
        qCritical() << "Backdoor setting file" << path << "is deleted";
        return QJsonObject();
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Unable to open backdoor setting file" << path;
        return QJsonObject();
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError jsonError;

    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

    if (jsonError.error != QJsonParseError::NoError)
    {
        qCritical() << "Error parsing json file" << path << jsonError.errorString();
        return QJsonObject();
    }

    QJsonObject json = doc.object();

    for (const QString& key : requiredKeys)
    {
        if (!json.contains(key))
        {
            qCritical() << "Backdoor json file" << path << "must contain key:" << key;
            return QJsonObject();
        }
    }

    return json;
}

void DeviceControllerCPP::processBackLightSettings(const QString &path)
{
    QVariantList data = mBacklightModelData;

    QJsonObject json = processJsonFile(path, {"red", "green", "blue", "mode", "on"});
    // if returned value is ok override the defaule values
    if (!json.isEmpty())
    {
        int r = json["red"].toInt();
        int g = json["green"].toInt();
        int b = json["blue"].toInt();
        int mode = json["mode"].toInt();
        bool on = json["on"].toBool();
        data = {r, g, b, mode, on};
    }

    setBacklight(data, true);
}

void DeviceControllerCPP::processFanSettings(const QString &path)
{
    auto speed = mFanSpeed;

    QJsonObject json = processJsonFile(path, {"speed"});
    // if returned value is ok override the defaule values
    if (!json.isEmpty())
    {
        speed = json["speed"].toInt();
    }

    _deviceIO->setFanSpeed(speed);
}

void DeviceControllerCPP::processBrightnessSettings(const QString &path)
{
    auto data = mSettingsModelData;

    QJsonObject json = processJsonFile(path, {"brightness"});
    // if returned value is ok override the defaule values
    if (!json.isEmpty())
    {
        data[0] = json["brightness"].toInt();
        data[3] = false;
    }

    _deviceIO->setSettings(data);
}

void DeviceControllerCPP::processRelaySettings(const QString &path)
{
    STHERM::RelayConfigs relays = Relay::instance()->relays();
    QJsonObject json = processJsonFile(path, {"o_b", "y1", "y2", "w1", "w2", "w3", "acc2", "acc1p", "acc1n", "g"});

    // if returned value is ok override the default values
    if (!json.isEmpty()) {
        //! stopping schemes from controlling relays
        mBackdoorSchemeEnabled = true;
        mTempScheme->stopSendingRelays();
        mHumidityScheme->stopSendingRelays();

        //! overrding values based on parsed data
        relays.g = (STHERM::RelayMode)json.value("g").toInt(2);
        relays.y1 = (STHERM::RelayMode)json.value("y1").toInt(2);
        relays.y2 = (STHERM::RelayMode)json.value("y2").toInt(2);
        relays.w1 = (STHERM::RelayMode)json.value("w1").toInt(2);
        relays.w2 = (STHERM::RelayMode)json.value("w2").toInt(2);
        relays.w3 = (STHERM::RelayMode)json.value("w3").toInt(2);
        relays.acc2 = (STHERM::RelayMode)json.value("acc2").toInt(2);
        relays.acc1p = (STHERM::RelayMode)json.value("acc1p").toInt(2);
        relays.acc1n = (STHERM::RelayMode)json.value("acc1n").toInt(2);
        relays.o_b  = (STHERM::RelayMode)json.value("o_b").toInt(2);

        SCHEME_LOG << "Update relays with backdoor. Relays: " << relays.printStr();
        _deviceIO->updateRelays(relays, true);

    } else if (mBackdoorSchemeEnabled) { // restore last state and get back to normal behavior restarting schemes
        SCHEME_LOG << "Update relays with Original. Relays: " << relays.printStr();
        _deviceIO->updateRelays(relays, true);
        mTempScheme->resumeSendingRelays();
        mHumidityScheme->resumeSendingRelays();
        mBackdoorSchemeEnabled = false;
    }
}

QByteArray DeviceControllerCPP::defaultSettings(const QString &path)
{
    if (path.endsWith("backlight.json")) {
        return m_default_backdoor_backlight;

    } else if (path.endsWith("fan.json")) {
        return m_default_backdoor_fan;

    } else if (path.endsWith("brightness.json")) {
        return m_default_backdoor_brightness;

    } else if (path.endsWith("relays.json")) {
        return m_default_backdoor_relays;

    } else {
        qWarning() << "Incompatible backdoor file, returning empty values";
    }

    return "";
}

void DeviceControllerCPP::writeSensorData(const QVariantMap& data) {
    auto directoryHasSpace = m_system->checkDirectorySpaces("/mnt/data/sensor");

    QString filePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/sensorData.csv";
    const QString dateTimeHeader = "DateTime UTC (sec)";

#ifdef __unix__
    filePath = "/mnt/data/sensor/sensorData.csv";
#endif

    const QStringList header = {dateTimeHeader, temperatureUIKey, humidityKey,
                                co2Key, etohKey, TvocKey,
                                iaqKey, pressureKey, RangeMilliMeterKey,
                                brightnessKey, fanSpeedKey};
    QFile file(filePath);

    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream out(&file);

        QString allData = out.readAll();
        file.resize(0);

        // Check the header
        auto checkHeader = allData.isEmpty() ? false : allData.split("\n").first().contains(dateTimeHeader);
        if (!checkHeader) {
            // Write header
            QStringList data;
            foreach (auto field, header) {
                data.append(field);
            }
            allData.append(data.join(",") + "\n");
        }

        // Write data rows
        QStringList dataStrList;
        dataStrList.append(QString::number(QDateTime::currentDateTimeUtc().toSecsSinceEpoch()));
        foreach (auto key, header) {
            if (key == dateTimeHeader)
                continue;

            dataStrList.append(QString::number(data.value(key, NAN).toDouble()));
        }

        allData.append(dataStrList.join(","));

        QStringList lines = allData.split("\n");

        // Remove the first data line from the file
        if (!directoryHasSpace && lines.count() > 1) {
            lines.remove(1);
        }

        foreach (auto line, lines) {
            out << line << "\n";
        }

        file.close();
        LOG_CHECK_DC(false) << "CSV file written successfully.";

    } else {
        LOG_DC << "Failed to open the file for writing/Reading." << directoryHasSpace;
    }
}

void DeviceControllerCPP::updateIsNeedOutdoorTemperature()
{
    auto isNeedOutdoorTemperature = mSystemSetup->systemType == AppSpecCPP::SystemType::DualFuelHeating && mSystemSetup->isAUXAuto;
    if (mIsNeedOutdoorTemperature == isNeedOutdoorTemperature) {
        return;
    }

    mIsNeedOutdoorTemperature = isNeedOutdoorTemperature;
    emit isNeedOutdoorTemperatureChanged();
}

bool DeviceControllerCPP::isNeedOutdoorTemperature() {
    return mIsNeedOutdoorTemperature;
}

void DeviceControllerCPP::updateIsEligibleOutdoorTemperature()
{
    auto isEligibleOutdoorTemperature = Device->hasClient() && !Device->serialNumber().isEmpty() && mDeviceHasInternet;
    if (mIsEligibleOutdoorTemperature == isEligibleOutdoorTemperature) {
        return;
    }

    mIsEligibleOutdoorTemperature = isEligibleOutdoorTemperature;
    emit isEligibleOutdoorTemperatureChanged();
}

bool DeviceControllerCPP::isEligibleOutdoorTemperature()
{
    return mIsEligibleOutdoorTemperature;
}

void DeviceControllerCPP::publishTestResults(const QString &resultsPath)
{
    const auto &config = _deviceAPI->deviceConfig();

    QString destinationIP = config.testConfigIp.empty()
                                ? "192.168.10.101"
                                : QString::fromStdString(config.testConfigIp);
    QString username = config.testConfigUser.empty()
                           ? "lucidtron1"
                           : QString::fromStdString(config.testConfigUser);
    QString password = config.testConfigPassword.empty()
                           ? "Tony6763"
                           : QString::fromStdString(config.testConfigPassword);
    QString destinationPath = config.testConfigDestination.empty()
                                  ? "d:/test_results/"
                                  : QString::fromStdString(config.testConfigDestination);

    LOG_DC << "start exporting test results as " << resultsPath << destinationIP << username
          << password << destinationPath;

    auto sent = m_system->sendResults(resultsPath,
                                      destinationIP,
                                      username,
                                      password,
                                      destinationPath);

    LOG_DC << "exporting test results ended " << sent;
}

void DeviceControllerCPP::doPerfTest(AppSpecCPP::SystemMode mode)
{
    if (!mSchemeDataProvider || mSchemeDataProvider->isPerfTestRunning()) {
        LOG_DC << "doPerfTest: Perf-test is already running";
        return;
    }

    mSchemeDataProvider->perfTestSystemMode(mode);
    mSchemeDataProvider->isPerfTestRunning(true);

    if (mTempScheme) {
        mTempScheme->restartWork(true);
    }

    if (mHumidityScheme) {
        mHumidityScheme->restartWork(true);
    }
}

void DeviceControllerCPP::revertPerfTest()
{
    if (!mSchemeDataProvider || !mSchemeDataProvider->isPerfTestRunning()) {
        LOG_DC << "revertPerfTest: No perf-test is running to revert";
        return;
    }

    mSchemeDataProvider->isPerfTestRunning(false);
    mSchemeDataProvider->perfTestSystemMode(AppSpecCPP::Off);

    if (mTempScheme) {
        mTempScheme->restartWork(true);
    }

    if (mHumidityScheme) {
        mHumidityScheme->restartWork(true);
    }
}

double DeviceControllerCPP::effectiveHumidity() {
    return mSchemeDataProvider->effectiveHumidity();
}
