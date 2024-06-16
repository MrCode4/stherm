#pragma once

#include <QObject>
#include <QThread>

#include "DeviceAPI.h"
#include "Relay.h"
#include "ScheduleCPP.h"
#include "SystemSetup.h"
#include "AppSpecCPP.h"


const double RELAYS_WAIT_MS = 500;

/*! ***********************************************************************************************
 * BaseScheme class to use in Humidity and temperature class.
 * ************************************************************************************************/

class BaseScheme : public QThread
{
    Q_OBJECT

public:
    explicit BaseScheme(DeviceAPI *deviceAPI, QObject *parent = nullptr);

    void setCanSendRelays(const bool& csr);

    virtual void setSystemSetup(SystemSetup *systemSetup);

    void setMainData(QVariantMap mainData);

signals:
    void stopWorkRequested();

    void sendRelayIsRunning(const bool& isRunning);
    void canSendRelay();

    //! Send relay to DeviceIOController and update relays into ti board.
    void updateRelays(STHERM::RelayConfigs);

    void currentTemperatureChanged();

    void currentHumidityChanged();

protected:
    //! To monitor data change: current Humidity, set Humidity, mode
    //! use low values for timeout in exit cases as it might had abrupt changes previously
    virtual int waitLoop(int timeout = 10000, AppSpecCPP::ChangeTypes overrideModes = AppSpecCPP::ChangeType::ctAll);

    //! Convert Celcius to Fahrenheit
    double toFahrenheit(double celsius);

protected:
    Relay*  mRelay;
    DeviceAPI *mDeviceAPI;
    SystemSetup *mSystemSetup = nullptr;
    ScheduleCPP* mSchedule = nullptr;

    STHERM::RelayConfigs lastConfigs;

    QVariantMap _mainData;

    //! Temperature parameters (Fahrenheit)
    double mCurrentTemperature = 20 * 1.8 + 32;

    //! Humidity parameters (Percentage)
    double mCurrentHumidity = 30;

    bool stopWork;

    bool mCanSendRelay;

    bool debugMode;
};

