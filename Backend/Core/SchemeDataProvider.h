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

    //! Set main data to update current temperature and current humidity
    void setMainData(QVariantMap mainData);

    //! The system setup, currentHumidity, currentTemperature, schedule, setPointTemperature
    //!  and vacation can only be changed through the DeviceController,
    //! ensuring that both the scheme and humidity scheme can only use it, not modify it directly.


    SystemSetup *systemSetup() const;
    void setSystemSetup(SystemSetup *systemSetup);

    double currentHumidity() const;

    double currentTemperature() const;

    ScheduleCPP *schedule() const;
    void setSchedule(ScheduleCPP *newSchedule);


    STHERM::Vacation vacation() const;
    void setVacation(const STHERM::Vacation &newVacation);

    void setSetPointTemperature(const double &newSetPointTemperature);
    double setPointTemperature() const;

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

    //! Fahrenheit
    double mSetPointTemperature;

    struct STHERM::Vacation mVacation;
};

