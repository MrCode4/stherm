#pragma once

#include <QObject>

#include "UtilityHelper.h"

/*! ***********************************************************************************************
 * THis class manage Vacation data.
 * todo: Add another properties.
 * ************************************************************************************************/

class Scheme : public QObject
{
    Q_OBJECT

public:
    Scheme(QObject *parent = nullptr);

    //! Update vacation mode
    STHERM::SystemMode updateVacationState(const struct STHERM::Vacation &vacation,
                                           const double &setTemperature,
                                           const double &currentTemperature,
                                           const double &currentHumidity);

    STHERM::SystemMode updateNormalState(const double &setTemperature,
                                         const double &currentTemperature,
                                         const double &currentHumidity);

    //! Start humidifier work
    void startHumidifierWork(int humidifier, QString device_state,
                             int humidity, int current_humidity,
                             int sth, int stl);

    //! Update current system state
    void setCurrentState(const int &humidifierId);

    STHERM::SystemMode getSysMode() const;
    void setSysMode(STHERM::SystemMode newSysMode);

    void setHumidifierState(bool on);
    void setDehumidifierState(bool on);

private:

    STHERM::SystemMode sysMode;

    int mHumidifierId;

};

