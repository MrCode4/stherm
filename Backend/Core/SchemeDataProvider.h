#pragma once

#include <QObject>
#include <QVariantMap>

#include "ScheduleCPP.h"
#include "SystemSetup.h"

/*! ***********************************************************************************************
 * This class provides data for Scheme and HumidityScheme.
 * ************************************************************************************************/

class SchemeDataProvider : public QObject
{
    Q_OBJECT

public:
    /* Public Constructors & Destructor
     * ****************************************************************************************/

    explicit SchemeDataProvider(QObject *parent = nullptr);

    /* Public Functions
     * ****************************************************************************************/

    void setMainData(QVariantMap mainData);

    SystemSetup *systemSetup() const;
    void setSystemSetup(SystemSetup *systemSetup);

    double currentHumidity() const;

    double currentTemperature() const;

    ScheduleCPP *schedule() const;
    void setSchedule(ScheduleCPP *newSchedule);


    STHERM::Vacation vacation() const;
    void setVacation(const STHERM::Vacation &newVacation);

signals:

    /* Public Signals
     * ****************************************************************************************/
    void currentHumidityChanged();
    void currentTemperatureChanged();
    void systemSetupChanged();
    void scheduleChanged();
    void vacationChanged();

private:
    /* Attributes
     * ****************************************************************************************/

    QVariantMap _mainData;

    SystemSetup *mSystemSetup = nullptr;

    ScheduleCPP* mSchedule = nullptr;

    //! Humidity parameters (Percentage)
    double mCurrentHumidity;

    //! Temperature parameters (Fahrenheit)
    double mCurrentTemperature;

    struct STHERM::Vacation mVacation;
};

