#include "DeviceControllerCPP.h"

#include "LogHelper.h"

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

    connect(_deviceIO, &DeviceIOController::mainDataReady, this, [this](QVariantMap data) {
        setMainData(data);
    });

    connect(m_scheme, &Scheme::changeBacklight, this, [this](QVariantList color, QVariantList afterColor) {

        TRACE << "Update backlight.";

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

    connect(_deviceIO,
            &DeviceIOController::alert,
            this,
            [this](STHERM::AlertLevel alertLevel,
                   STHERM::AlertTypes alertType,
                   QString alertMessage) {
                emit alert(alertLevel, alertType, alertMessage);
            });
}

DeviceControllerCPP::~DeviceControllerCPP() {}

bool DeviceControllerCPP::setBacklight(QVariantList data, bool isScheme)
{
    if (!isScheme) {
        mBacklightModelData = data;
    }

    return _deviceIO->setBacklight(data);
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
    m_scheme->setSetPointTemperature(temperature);
}

void DeviceControllerCPP::setRequestedHumidity(const double humidity)
{
    m_scheme->setRequestedHumidity(humidity);
}

void DeviceControllerCPP::startDevice()
{
    //! todo: move to constructor later
    _deviceIO->createConnections();

    TRACE << "start mode is: " << _deviceAPI->getStartMode();

    // Satart with delay to ensure the model loaded.
    QTimer::singleShot(5000, this, [this]() {
        TRACE << "starting scheme";
        m_scheme->restartWork();
    });
}

void DeviceControllerCPP::stopDevice()
{
    _deviceIO->stopReading();
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
    if (_mainData == mainData)
        return;

    _mainData = mainData;

    if (m_scheme)
        m_scheme->setMainData(mainData);

}

QVariantMap DeviceControllerCPP::getMainData()
{
    return _mainData;
}
