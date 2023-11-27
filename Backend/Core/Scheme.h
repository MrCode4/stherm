#pragma once

#include <QThread>
#include <QVariantMap>

#include "Core/Relay.h"
#include "UtilityHelper.h"
#include "include/timing.h"

/*! ***********************************************************************************************
 * THis class manage Vacation data.
 * todo: Add another properties.
 * ************************************************************************************************/

enum ChangeType {
    CurrentTemperature = 0,
    SetTemperature,
    Mode,

};

class Scheme : public QThread
{
    Q_OBJECT

public:
    explicit Scheme(QObject *parent = nullptr);

    ~Scheme();

    void updateRealState(const struct STHERM::Vacation &vacation,
                         const double &setTemperature,
                         const double &currentTemperature,
                         const double &currentHumidity);

    //! Start humidifier work
    void startHumidifierWork(int humidifier, QString device_state,
                             int humidity, int current_humidity,
                             int sth, int stl);

    //! Update current system state
    void setCurrentState(const int &humidifierId);

    STHERM::SystemMode getCurrentSysMode() const;
    void setCurrentSysMode(STHERM::SystemMode newSysMode);

    void setHumidifierState(bool on);
    void setDehumidifierState(bool on);

    STHERM::SystemMode realSysMode() const;
    void setRealSysMode(STHERM::SystemMode newRealSysMode);

    void startWork();

    void setMainData(QVariantMap mainData);

signals:
    void changeBacklight(QVariantList colorData);

    void alert();

    void currentTemperatureChanged();
    void setTemperatureChanged();

    void modeChanged();

protected:
    virtual void run();

private:
    //! Update vacation mode
    STHERM::SystemMode updateVacationState(const struct STHERM::Vacation &vacation,
                                           const double &setTemperature,
                                           const double &currentTemperature,
                                           const double &currentHumidity);

    STHERM::SystemMode updateNormalState(const double &setTemperature,
                                         const double &currentTemperature,
                                         const double &currentHumidity);

    void coolingHeatPumpRole1(bool needToWait = true);
    void coolingHeatPumpRole2();
    void heatingEmergencyHeatPumpRole1();
    void heatingEmergencyHeatPumpRole2();
    void heatingHeatPumpRole1();
    void heatingEmergencyHeatPumpRole3();
    void heatingHeatPumpRole2(bool needToWait = true);
    void heatingHeatPumpRole3();
    void heatingConventionalRole1(bool needToWait = true);
    void heatingConventionalRole2();
    void heatingConventionalRole3();

    // To monitor data change: current temperature, set temperature, mode
    int waitLoop();

private:
    /* Attributes
     * ****************************************************************************************/
    QVariantMap _mainData;

    STHERM::SystemMode mCurentSysMode;
    STHERM::SystemMode mRealSysMode;

    STHERM::CoolingType mDeviceType;

    Timing* mTiming;
    Relay*  mRelay;

    int mHumidifierId;

    double mCurrentTemperature;
    double mSetPointTemperature;

    bool stopWork;
};

