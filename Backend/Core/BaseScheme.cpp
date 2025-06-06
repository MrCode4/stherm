#include "BaseScheme.h"

#include "LogCategoires.h"
#include "SchemeDataProvider.h"

BaseScheme::BaseScheme(DeviceAPI* deviceAPI, QSharedPointer<SchemeDataProvider> sharedData, QObject *parent) :
    mDeviceAPI(deviceAPI),
    stopWork(true),
    mCanSendRelay(true),
    mIsSendingRelay(false),
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
    connect(mDataProvider.data(), &SchemeDataProvider::scheduleChanged, this, &BaseScheme::onScheduleChanged);
    connect(mDataProvider.data(), &SchemeDataProvider::setTemperatureChanged, this, &BaseScheme::setTemperatureChanged);
    connect(mDataProvider.data(), &SchemeDataProvider::setHumidityChanged, this, &BaseScheme::setHumidityChanged);
}


void BaseScheme::setCanSendRelays(const bool &csr)
{
    mCanSendRelay = csr;

    if (mCanSendRelay) {
        emit canSendRelay();
    }
}

void BaseScheme::setIsSendingRelays(bool sending)
{
    mIsSendingRelay = sending;
    emit sendRelayIsRunning(sending);
}

void BaseScheme::stopSendingRelays()
{
    SCHEME_LOG << this << "Sending relays stopping.";
    mCanSendRelay = false;
    if (isSendingRelay()){
        SCHEME_LOG << this << "Waiting to finish Ongoing.";
        waitLoop(-1, AppSpecCPP::ctSendRelay);
        SCHEME_LOG << this << "Finished.";
    }
}

void BaseScheme::resumeSendingRelays()
{
    mCanSendRelay = true;
    emit canSendRelay(true);
}

bool BaseScheme::isSendingRelay()
{
    return mIsSendingRelay;
}

void BaseScheme::onScheduleChanged()
{
    SCHEME_LOG << "restarting as the schedule is changed";

    restartWork();
}

int BaseScheme::waitLoop(int timeout, AppSpecCPP::ChangeTypes overrideModes)
{
    return 0;
}
