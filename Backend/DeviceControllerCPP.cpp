#include "DeviceControllerCPP.h"

#include "LogHelper.h"

/* ************************************************************************************************
 * Constructors & Destructor
 * ************************************************************************************************/
DeviceControllerCPP::DeviceControllerCPP(QObject *parent)
    : QObject(parent)
    , _deviceIO(new DeviceIOController(this))
    , _deviceAPI(new DeviceAPI(this))
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
    _mainData = mainDataMap;

    LOG_DEBUG("TEST");
    connect(_deviceIO, &DeviceIOController::mainDataReady, this, [this](QVariantMap data) {
        _mainData = data;
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

bool DeviceControllerCPP::setBacklight(QVariantList data)
{
    return _deviceIO->setBacklight(data);
}

bool DeviceControllerCPP::setSettings(QVariantList data)
{
    return _deviceIO->setSettings(data);
}

void DeviceControllerCPP::startDevice()
{
    //! todo: move to constructor later
    _deviceIO->createConnections();

    _deviceIO->setStopReading(false);

    TRACE << "start mode is: " << _deviceAPI->getStartMode();
}

void DeviceControllerCPP::stopDevice()
{
    _deviceIO->setStopReading(true);
}

QVariantMap DeviceControllerCPP::getMainData()
{
    return _mainData;
}
