#pragma once

#include <QObject>
#include <QVariantMap>

#include "ScheduleCPP.h"
#include "SystemSetup.h"
#include "Property.h"

/*! ***********************************************************************************************
 * This class provides data for Scheme and HumidityScheme.
 * ************************************************************************************************/

class SchemeDataProvider : public QObject
{
    Q_OBJECT
    PROPERTY_PUB_DEF_VAL(bool, isPerfTestRunning, false)
    PROPERTY_PUB_DEF_VAL(AppSpecCPP::SystemMode, perfTestSystemMode, AppSpecCPP::Off)

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

    void setOutdoorTemperature(double temp);

    STHERM::Vacation vacation() const;
    void setVacation(const STHERM::Vacation &newVacation);

    void setSetPointTemperature(const double &newSetPointTemperature);
    double setPointTemperature() const;

    void setRequestedHumidity(const double &setPointHumidity);
    double setPointHumidity() const;

    //! Set minimum of auto temperature ranges
    //! cel_value: Celsius
    void setAutoMinReqTemp(const double &cel_value);
    //! Fahrenheit
    double autoMinReqTempF() const;

    //! Set maximum of auto temperature ranges
    //! cel_value: Celsius
    void setAutoMaxReqTemp(const double &cel_value);
    //! Fahrenheit
    double autoMaxReqTempF() const;

    AppSpecCPP::AccessoriesType getAccessoriesType() const;

    AppSpecCPP::AccessoriesWireType getAccessoriesWireType() const;

    //! Calculate the effective temperature based on
    //! Current temperature, vacation, schedule, system mode and
    //! current system mode (relay state)
    //! The priority order for calculations is vacation, schedule, and then system mode.
    //! Ensure all temperature attributes involved in this calculation
    //! are expressed in Fahrenheit, and the final effective temperature
    //! result should be in Fahrenheit as well.
    double effectiveTemperature() const;

    bool isVacationEffective() const;

    AppSpecCPP::SystemMode effectiveSystemMode() const;

    void setAutoMinReqTempF(const double &fah_value);

    void setAutoMaxReqTempF(const double &fah_value);

    double outdoorTemperatureF() const;

    double dualFuelThresholdF() const;

    //! Since the heat pump and cool stages both determine the Y wires, they're essentially the same.
    int heatPumpStage() const;

    double effectiveEmergencyHeatingThresholdF();

    bool  isRelaysInitialized();
    void setIsRelaysInitialized(const bool &isRelaysInitialized);

     //! Return the effective humidity
    double effectiveSetHumidity() const;

signals:

    /* Public Signals
     * ****************************************************************************************/
    void currentHumidityChanged();
    void currentTemperatureChanged();
    void systemSetupChanged();
    void scheduleChanged();
    void vacationChanged();
    void setTemperatureChanged();
    void setHumidityChanged();

    void outdoorTemperatureReady();
    void outdoorTemperatureChanged();

    void monitoringTemperatureUpdated(double monitoringTempratureC) const;
    void effectiveHumidityChanged(double effectiveHumidity) const;

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

    double mSetPointHumidity;

    struct STHERM::Vacation mVacation;

    //! Auto mode properites (Fahrenheit)
    double mAutoMinReqTempF;
    double mAutoMaxReqTempF;

    //! Outdoot temperature
    //! Celsius
    double mOutdoorTemperature;

    bool mIsRelaysInitialized;

};

