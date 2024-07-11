#pragma once

#include <QObject>
#include <QThread>

#include "DeviceAPI.h"
#include "Relay.h"
#include "ScheduleCPP.h"
#include "SystemSetup.h"
#include "AppSpecCPP.h"


const double RELAYS_WAIT_MS = 500;

class SchemeDataProvider;

/*! ***********************************************************************************************
 * BaseScheme class to use in Humidity and temperature class.
 * TODO: Move some Scheme attributes to SchemeDataProvider (e.g. mVacationMinimumTemperature,
 * mVacationMaximumTemperature, mAutoMaxReqTemp, ...)
 * ************************************************************************************************/

class BaseScheme : public QThread
{
    Q_OBJECT

public:
    explicit BaseScheme(DeviceAPI *deviceAPI, QSharedPointer<SchemeDataProvider> sharedData,
                        QObject *parent = nullptr);


    virtual void setSystemSetup() = 0;

    //! Restart the worker thread
    virtual void restartWork(bool forceStart = false) = 0;

    virtual void setVacation() = 0;

    void setCanSendRelays(const bool& csr);

    void onScheduleChanged();

signals:
    void stopWorkRequested();

    void sendRelayIsRunning(const bool& isRunning);
    void canSendRelay();

    //! Send relay to DeviceIOController and update relays into ti board.
    void updateRelays(STHERM::RelayConfigs);

    void currentTemperatureChanged();

    void setTemperatureChanged();

    void currentHumidityChanged();

    void setHumidityChanged();

protected:
    //! To monitor data change: current Humidity, set Humidity, mode
    //! use low values for timeout in exit cases as it might had abrupt changes previously
    virtual int waitLoop(int timeout = 10000, AppSpecCPP::ChangeTypes overrideModes = AppSpecCPP::ChangeType::ctAll);

    //! Return the effective humidity
    double effectiveSetHumidity() const;

protected:
    QSharedPointer<SchemeDataProvider> mDataProvider;

    Relay*  mRelay;
    DeviceAPI *mDeviceAPI;

    bool stopWork;

    bool mCanSendRelay;

    bool debugMode;
};
