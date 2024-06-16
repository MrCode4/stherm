#include "BaseScheme.h"

BaseScheme::BaseScheme(DeviceAPI* deviceAPI, QObject *parent) :
    mDeviceAPI(deviceAPI),
    stopWork(true),
    mCanSendRelay(true),
    debugMode(true),
    QThread{parent}
{
    mRelay = Relay::instance();

    debugMode = RELAYS_WAIT_MS != 0;
}


void BaseScheme::setCanSendRelays(const bool &csr)
{
    mCanSendRelay = csr;

    if (mCanSendRelay) {
        emit canSendRelay();
    }
}

void BaseScheme::setSystemSetup(SystemSetup *systemSetup)
{
    if (!systemSetup || mSystemSetup == systemSetup)
        return;

    if (mSystemSetup) {
        mSystemSetup->disconnect(this);
    }
}

void BaseScheme::setMainData(QVariantMap mainData)
{
    if (_mainData == mainData)
        return;

    _mainData = mainData;

    bool isOk;
    double tc = mainData.value("temperature").toDouble(&isOk);
    double currentTemp = toFahrenheit(tc);

    if (isOk) {
        // meaningful change
        if (qAbs(currentTemp - mCurrentTemperature) > 0.1)
            emit currentTemperatureChanged();
        mCurrentTemperature = currentTemp;
    }

    double currentHumidity = mainData.value("humidity").toDouble(&isOk);

    if (isOk && qAbs(currentHumidity - mCurrentHumidity) > 0.1) {
        mCurrentHumidity = currentHumidity;
        emit currentHumidityChanged();
    }
}

int BaseScheme::waitLoop(int timeout, AppSpecCPP::ChangeTypes overrideModes)
{
    return 0;
}

double BaseScheme::toFahrenheit(double celsius) {
    return celsius * 9 / 5 + 32.0;
}
