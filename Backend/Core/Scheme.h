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
    STHERM::SystemMode updateVacation(const struct STHERM::Vacation &vacation,
                                      const double &setTemperature,
                                      const double &currentTemperature,
                                      const double &currentHumidity);

    STHERM::SystemMode getSysMode() const;
    void setSysMode(STHERM::SystemMode newSysMode);

    void setHumidifierState(bool on);
    void setDehumidifierState(bool on);

private:

    STHERM::SystemMode sysMode;

};

