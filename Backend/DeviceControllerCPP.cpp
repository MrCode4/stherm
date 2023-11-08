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

    qDebug() << Q_FUNC_INFO << __LINE__ << _deviceController->getStartMode(90);
    if (className == "system") {
        if (method == "getMainData") {
            return getMainData();
        }
    }

    return {};
}

void DeviceControllerCPP::startDevice()
{
    _deviceController->createConnections();
}

QVariantMap DeviceControllerCPP::getMainData()
{
    return _mainData;
}
