#include "DeviceControllerCPP.h"

#include "LogHelper.h"

/* ************************************************************************************************
 * Constructors & Destructor
 * ************************************************************************************************/
DeviceControllerCPP::DeviceControllerCPP(QObject *parent)
    : QObject(parent)
    , _deviceController(new DeviceIOController)
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
    connect(_deviceController, &DeviceIOController::mainDataReady, this, [this](QVariantMap data) {
        _mainData = data;
    });

    connect(_deviceController, &DeviceIOController::alert, this, [this](STHERM::AlertLevel alertLevel,
                                                                        STHERM::AlertTypes alertType,
                                                                        QString alertMessage) {
        TRACE << alertLevel << alertType << alertMessage;
        emit alert(alertLevel, alertType,  alertMessage);
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

    _deviceController->sendRequest(className, method, data);

    return {};
}

void DeviceControllerCPP::startDevice()
{
    //! todo: move to constructor later
    _deviceController->createConnections();

    _deviceController->setStopReading(false);
}

void DeviceControllerCPP::stopDevice()
{
    _deviceController->setStopReading(true);
}

QVariantMap DeviceControllerCPP::getMainData()
{
    return _mainData;
}
