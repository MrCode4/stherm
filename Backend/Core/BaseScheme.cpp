#include "BaseScheme.h"

#include "SchemeDataProvider.h"

BaseScheme::BaseScheme(DeviceAPI* deviceAPI, QSharedPointer<SchemeDataProvider> sharedData, QObject *parent) :
    mDeviceAPI(deviceAPI),
    stopWork(true),
    mCanSendRelay(true),
    debugMode(true),
    mDataProvider(sharedData),
    QThread{parent}
{
    mRelay = Relay::instance();

    debugMode = RELAYS_WAIT_MS != 0;

    connect(mDataProvider.data(), &SchemeDataProvider::currentHumidityChanged, this, &BaseScheme::currentHumidityChanged);
    connect(mDataProvider.data(), &SchemeDataProvider::currentTemperatureChanged, this, &BaseScheme::currentTemperatureChanged);
    connect(mDataProvider.data(), &SchemeDataProvider::systemSetupChanged, this, &BaseScheme::setSystemSetup);
    connect(mDataProvider.data(), &SchemeDataProvider::vacationChanged, this, &BaseScheme::setVacation);
    connect(mDataProvider.data(), &SchemeDataProvider::scheduleChanged, this, &BaseScheme::restartWork);
}


void BaseScheme::setCanSendRelays(const bool &csr)
{
    mCanSendRelay = csr;

    if (mCanSendRelay) {
        emit canSendRelay();
    }
}

int BaseScheme::waitLoop(int timeout, AppSpecCPP::ChangeTypes overrideModes)
{
    return 0;
}
