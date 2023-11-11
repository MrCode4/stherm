#include "DeviceControllerCPP.h"

/* ************************************************************************************************
 * Constructors & Destructor
 * ************************************************************************************************/
DeviceControllerCPP::DeviceControllerCPP(QObject *parent)
    : QObject(parent)
    , _deviceController(new DeviceIOController)
{
    _mainData = {{"temp", QVariant(0)}, {"hum", QVariant(0)}};

    connect(_deviceController, &DeviceIOController::dataReady, this, [this](QVariantMap data) {
        _mainData = data;
    });
}

DeviceControllerCPP::~DeviceControllerCPP() {}

QVariantMap DeviceControllerCPP::sendRequest(QString className, QString method, QVariantList data)
{
    _deviceController->sendRequest(className, method, data);

    if (className == "system") {
        if (method == "getMainData") {
            return getMainData();
        }
    }

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
