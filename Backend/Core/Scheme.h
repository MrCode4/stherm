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
                                      const double &temperature);

    STHERM::SystemMode getSysMode() const;
    void setSysMode(STHERM::SystemMode newSysMode);

private:

    STHERM::SystemMode sysMode;

};

