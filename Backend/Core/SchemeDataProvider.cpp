#include "SchemeDataProvider.h"

#include "LogHelper.h"
#include "UtilityHelper.h"
#include "Relay.h"

SchemeDataProvider::SchemeDataProvider(QObject *parent) :
    mOutdoorTemperature(25.0),
    mIsRelaysInitialized(false),
    QObject{parent}
{
}

void SchemeDataProvider::setMainData(QVariantMap mainData)
{
    if (_mainData == mainData)
        return;

    _mainData = mainData;

    bool isOk;
    double tc = mainData.value(roundTemperatureKey).toDouble(&isOk);

    if (isOk) {
        double currentTemp = UtilityHelper::toFahrenheit(tc);
        // meaningful change
        if (qAbs(currentTemp - mCurrentTemperature) > 0.1) {
            mCurrentTemperature = currentTemp;
            emit currentTemperatureChanged();
        }
    }

    double currentHumidity = mainData.value(humidityKey).toDouble(&isOk);

    if (isOk && qAbs(currentHumidity - mCurrentHumidity) > 0.1) {
        mCurrentHumidity = currentHumidity;
        emit currentHumidityChanged();
    }
}

void SchemeDataProvider::setSystemSetup(SystemSetup *systemSetup)
{
    TRACE << systemSetup << mSystemSetup;
    if (!systemSetup || mSystemSetup == systemSetup)
        return;

    if (mSystemSetup) {
        mSystemSetup->disconnect(this);
    }

     mSystemSetup = systemSetup;

    emit systemSetupChanged();
}

AppSpecCPP::AccessoriesType SchemeDataProvider::getAccessoriesType() const
{
    return mSystemSetup->systemAccessories->getAccessoriesType();
}

AppSpecCPP::AccessoriesWireType SchemeDataProvider::getAccessoriesWireType() const
{
    return mSystemSetup->systemAccessories->getAccessoriesWireType();
}

bool SchemeDataProvider::isVacationEffective() const
{
    return isPerfTestRunning() ? false : systemSetup()->isVacation;
}

AppSpecCPP::SystemMode SchemeDataProvider::effectiveSystemMode() const
{
    if (isPerfTestRunning()) {
        return perfTestSystemMode();
    }

    return systemSetup()->systemMode;
}

double SchemeDataProvider::effectiveTemperature() const
{
    double effTemperature = setPointTemperature();
    double monitoringTemprature = effTemperature;

    if (isPerfTestRunning()) {
        effTemperature = perfTestSystemMode() == AppSpecCPP::Heating ? 90 : 40;
        monitoringTemprature = effTemperature;

    } else if (isVacationEffective()) {
        //! Vacation properites (Fahrenheit)
        double minimumTemperature = UtilityHelper::toFahrenheit(mVacation.minimumTemperature);
        double maximumTemperature = UtilityHelper::toFahrenheit(mVacation.maximumTemperature);

        if ((minimumTemperature - currentTemperature()) > 0.001) {
            effTemperature = minimumTemperature;

        } else if ((maximumTemperature - currentTemperature()) < 0.001) {
            effTemperature = maximumTemperature;
        }

         monitoringTemprature = (minimumTemperature + maximumTemperature) / 2;

    } else if (schedule() || effectiveSystemMode() == AppSpecCPP::SystemMode::Auto) {

        // The mode can be heating or cooling
        // In off mode schedule() is null
        if (schedule() && effectiveSystemMode() != AppSpecCPP::SystemMode::Auto) {
            effTemperature = UtilityHelper::toFahrenheit(schedule()->effectiveTemperature(effectiveSystemMode()));
            monitoringTemprature = effTemperature;

        } else {

            // Use auto mode values by default and change later in schedule if needed.
            double minReqTemp = mAutoMinReqTempF;
            double maxReqTemp = mAutoMaxReqTempF;

            // Use schedule and use the auto mode logics.
            if (schedule()) {
                minReqTemp = UtilityHelper::toFahrenheit(schedule()->minimumTemperature);
                maxReqTemp = UtilityHelper::toFahrenheit(schedule()->maximumTemperature);
            }

            monitoringTemprature = (minReqTemp + maxReqTemp) / 2;

            if ((minReqTemp - currentTemperature()) > 0.001) {
                effTemperature = minReqTemp;

            } else if ((maxReqTemp - currentTemperature()) < 0.001) {
                effTemperature = maxReqTemp;

            } else {
                auto relay = Relay::instance();

                // Set the effective temperature to the boundary temperature to shutdown the system
                // todo: Manage the Emergency mode.
                if (relay->currentState() == AppSpecCPP::SystemMode::Cooling)
                    effTemperature = maxReqTemp;
                else if (relay->currentState() == AppSpecCPP::SystemMode::Heating)
                    effTemperature = minReqTemp;
                else
                    effTemperature = currentTemperature();
            }
        }
    }

    emit monitoringTemperatureUpdated(UtilityHelper::toCelsius(monitoringTemprature));

    return effTemperature;
}

double SchemeDataProvider::effectiveHumidity()
{
    double effectiveHumidity = setPointHumidity();

    if (isVacationEffective()) {
        if (getAccessoriesType() == AppSpecCPP::AccessoriesType::Humidifier) {
            effectiveHumidity = mVacation.minimumHumidity;
            if ((mVacation.minimumHumidity - currentHumidity()) > 0.001) {
                effectiveHumidity = mVacation.maximumHumidity;
            }

        } else if (getAccessoriesType() == AppSpecCPP::AccessoriesType::Dehumidifier) {
            effectiveHumidity = mVacation.maximumHumidity;
            if (currentHumidity() - mVacation.maximumHumidity > 0.001) {
                effectiveHumidity = mVacation.minimumHumidity;
            }
        }

    } else if (mSchedule) {
        //! TODO
    }

    emit effectiveHumidityChanged(effectiveHumidity);

    return effectiveHumidity;
}

void SchemeDataProvider::setAutoMinReqTempF(const double &fah_value)
{
    if (qAbs(mAutoMinReqTempF - fah_value) > 0.001)
        mAutoMinReqTempF = fah_value;
}

void SchemeDataProvider::setAutoMinReqTemp(const double &cel_value)
{
    auto minF = UtilityHelper::toFahrenheit(cel_value);
    setAutoMinReqTempF(minF);
}

double SchemeDataProvider::autoMinReqTempF() const
{
    return mAutoMinReqTempF;
}

void SchemeDataProvider::setAutoMaxReqTemp(const double &cel_value)
{
    auto maxF = UtilityHelper::toFahrenheit(cel_value);
    setAutoMaxReqTempF(maxF);
}

void SchemeDataProvider::setAutoMaxReqTempF(const double& fah_value)
{
    if (qAbs(mAutoMaxReqTempF - fah_value) > 0.001)
        mAutoMaxReqTempF = fah_value;
}

double SchemeDataProvider::outdoorTemperatureF() const
{
    return UtilityHelper::toFahrenheit(mOutdoorTemperature);
}

double SchemeDataProvider::dualFuelThresholdF() const
{
    return UtilityHelper::toFahrenheit(mSystemSetup->dualFuelThreshold);
}

int SchemeDataProvider::heatPumpStage() const
{
    return mSystemSetup->coolStage;
}

double SchemeDataProvider::effectiveEmergencyHeatingThresholdF()
{
    // Default is manual emergency mode (1 F).
    auto  effThreshold = -1.0;
    return effThreshold;
}

bool SchemeDataProvider::isRelaysInitialized() {
    return mIsRelaysInitialized;
}

void SchemeDataProvider::setIsRelaysInitialized(const bool &isRelaysInitialized) {
    mIsRelaysInitialized = isRelaysInitialized;
}

double SchemeDataProvider::autoMaxReqTempF() const
{
    return mAutoMaxReqTempF;
}

void SchemeDataProvider::setSchedule(ScheduleCPP *newSchedule)
{
    if (mSchedule == newSchedule)
        return;

    mSchedule = newSchedule;

    emit scheduleChanged();
}

void SchemeDataProvider::setOutdoorTemperature(double temp)
{
    bool changed = mOutdoorTemperature != temp;

    mOutdoorTemperature = temp;
    emit outdoorTemperatureReady();

    if (changed)
        emit outdoorTemperatureChanged();
}

double SchemeDataProvider::currentHumidity() const
{
    return mCurrentHumidity;
}

double SchemeDataProvider::currentTemperature() const
{
    return mCurrentTemperature;
}

ScheduleCPP *SchemeDataProvider::schedule() const
{
    return mSchedule;
}

SystemSetup *SchemeDataProvider::systemSetup() const
{
    return mSystemSetup;
}

STHERM::Vacation SchemeDataProvider::vacation() const
{
    return mVacation;
}

void SchemeDataProvider::setVacation(const STHERM::Vacation &newVacation)
{
    if (mVacation == newVacation)
        return;

    mVacation = newVacation;
    emit vacationChanged();
}

void SchemeDataProvider::setSetPointTemperature(const double& newSetPointTemperature)
{
    auto newSetPointTemperatureF = UtilityHelper::toFahrenheit(newSetPointTemperature);
    if (qAbs(mSetPointTemperature - newSetPointTemperatureF) < 0.001)
        return;

    mSetPointTemperature = newSetPointTemperatureF;
    emit setTemperatureChanged();

    TRACE << "SetPointTemperature (F) changed to " << mSetPointTemperature;
}

double SchemeDataProvider::setPointTemperature() const
{
    return mSetPointTemperature;
}

double SchemeDataProvider::effectiveSetHumidity() const
{
    double effHumidity = setPointHumidity();

    if (isPerfTestRunning()) {
        return effHumidity;
    }

    // will not happen for now, in vacation it is handled internally
    if (isVacationEffective()) {
        double vacationMinimumHumidity =vacation().minimumHumidity;
        double vacationMaximumHumidity = vacation().maximumHumidity;

        if ((vacationMinimumHumidity - mCurrentHumidity) > 0.001) {
            effHumidity  = vacationMinimumHumidity;

        } else if ((vacationMaximumHumidity - mCurrentHumidity) < 0.001) {
            effHumidity  = vacationMaximumHumidity;
        }

    } else if (schedule()) {
        effHumidity  = schedule()->humidity;

    }

    return effHumidity;
}


void SchemeDataProvider::setRequestedHumidity(const double &setPointHumidity)
{
    if (qAbs(mSetPointHumidity - setPointHumidity) < 0.001) {
        return;
    }

    mSetPointHumidity = setPointHumidity;
    emit setHumidityChanged();

    TRACE << "SetPointHumidity (%) changed to " << mSetPointHumidity;
}

double SchemeDataProvider::setPointHumidity() const
{
    return mSetPointHumidity;
}
