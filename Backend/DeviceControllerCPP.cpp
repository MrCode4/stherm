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


    LOG_DEBUG("TEST");
    connect(_deviceIO, &DeviceIOController::mainDataReady, this, [this](QVariantMap data) {
        setMainData(data);
    });

    connect(m_scheme, &Scheme::changeBacklight, this, [this](QVariantList data, int secs) {
        setBacklight(data, true);

        if (secs < 0)
            return;

        // Back to last backlight after secs seconds
        QTimer timer;

        timer.connect(&timer, &QTimer::timeout, this, [this]() {
            setBacklight(mBacklightModelData, true);
        });

        timer.start(secs * 1000);
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

QVariantMap DeviceControllerCPP::sendRequest(QString className, QString method, QVariantList data)
{
    if (className == "system") {
        if (method == "getMainData") {
            return getMainData();
        }
    }

    return {};
}

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

void DeviceControllerCPP::setRequestedTemperature(const double temperature)
{
    m_scheme->setSetPointTemperature(temperature);
}

void DeviceControllerCPP::startDevice()
{
    //! todo: move to constructor later
    _deviceIO->createConnections();

    _deviceIO->setStopReading(false);

    TRACE << "start mode is: " << _deviceAPI->getStartMode();

    m_scheme->start();
}

void DeviceControllerCPP::stopDevice()
{
    _deviceIO->setStopReading(true);
}

SystemSetup *DeviceControllerCPP::systemSetup() const {
    return mSystemSetup;
}

void DeviceControllerCPP::setSystemSetup(SystemSetup *systemSetup) {
    if (mSystemSetup == systemSetup)
        return;

    mSystemSetup = systemSetup;

    qDebug() << Q_FUNC_INFO << __LINE__ <<m_scheme;
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
