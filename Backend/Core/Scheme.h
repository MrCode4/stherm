#pragma once

#include <QThread>
#include <QVariantMap>

#include "UtilityHelper.h"

/*! ***********************************************************************************************
 * THis class manage Vacation data.
 * todo: Add another properties.
 * ************************************************************************************************/

class Scheme : public QThread
{
    Q_OBJECT

public:
    Scheme(QObject *parent = nullptr);

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

    void startWork2();

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
private:
    /* Attributes
     * ****************************************************************************************/
    QVariantMap _mainData;

    STHERM::SystemMode mCurentSysMode;
    STHERM::SystemMode mRealSysMode;

    STHERM::CoolingType mDeviceType;

    int mHumidifierId;

    double mCurrentTemperature;

    bool stopWork;
    void startWork3();
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
};

