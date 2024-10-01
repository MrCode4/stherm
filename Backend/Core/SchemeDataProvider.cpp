#include "SchemeDataProvider.h"

#include "LogHelper.h"
#include "UtilityHelper.h"
#include "Relay.h"

SchemeDataProvider::SchemeDataProvider(NUVE::Sync *sync, QObject *parent) :
    mSync(sync),
    mOutdoorTemperature(35.0),
    QObject{parent}
{
    mGetOutdoorTemperatureTimer.setInterval(60 * 1000);
    mGetOutdoorTemperatureTimer.setSingleShot(false);
    connect(&mGetOutdoorTemperatureTimer, &QTimer::timeout, this, [this]() {
        mSync->getOutdoorTemperature();
    });

    mOutdoorTemperature = -1;
    connect(mSync, &NUVE::Sync::outdoorTemperatureReady, this, [this](bool success, double temp) {
        TRACE << "Outdoor temperature:" << success << temp;
        if (success) {
            if (mOutdoorTemperature != temp) {
                mOutdoorTemperature = temp;
                emit outdoorTemperatureChanged();
            }

            if (systemSetup()->systemType != AppSpecCPP::DualFuelHeating) {
                mGetOutdoorTemperatureTimer.stop();
            }
        }
    });
}

void SchemeDataProvider::setMainData(QVariantMap mainData)
{
    if (_mainData == mainData)
        return;

    _mainData = mainData;

    bool isOk;
    double tc = mainData.value("temperature").toDouble(&isOk);
    double currentTemp = UtilityHelper::toFahrenheit(tc);

    if (isOk) {
        // meaningful change
        if (qAbs(currentTemp - mCurrentTemperature) > 0.1)
            emit currentTemperatureChanged();
        mCurrentTemperature = currentTemp;
    }

    double currentHumidity = mainData.value("humidity").toDouble(&isOk);

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

     // To provide outdoor temperature
    connect(mSystemSetup, &SystemSetup::systemTypeChanged, this, [=] {
         if (mSystemSetup->systemType == AppSpecCPP::SystemType::DualFuelHeating) {
             mSync->getOutdoorTemperature();
             mGetOutdoorTemperatureTimer.start();

         } else {
             mGetOutdoorTemperatureTimer.stop();
         }
    });

    emit systemSetupChanged();

    // To cache the outdoor temperature, it will be stop when get data successfully in the other system types
    mGetOutdoorTemperatureTimer.start();
}

AppSpecCPP::AccessoriesType SchemeDataProvider::getAccessoriesType() const
{
    return mSystemSetup->systemAccessories->getAccessoriesType();
}

AppSpecCPP::AccessoriesWireType SchemeDataProvider::getAccessoriesWireType() const
{
    return mSystemSetup->systemAccessories->getAccessoriesWireType();
}

double SchemeDataProvider::effectiveTemperature() const
{
    double effTemperature = setPointTemperature();

    if (systemSetup()->isVacation) {
        //! Vacation properites (Fahrenheit)
        double minimumTemperature = UtilityHelper::toFahrenheit(mVacation.minimumTemperature);
        double maximumTemperature = UtilityHelper::toFahrenheit(mVacation.maximumTemperature);

        if ((minimumTemperature - currentTemperature()) > 0.001) {
            effTemperature = minimumTemperature;

        } else if ((maximumTemperature - currentTemperature()) < 0.001) {
            effTemperature = maximumTemperature;
        }

    } else if (schedule() || systemSetup()->systemMode == AppSpecCPP::SystemMode::Auto) {

        // The mode can be heating or cooling
        // In off mode schedule() is null
        if (schedule() && systemSetup()->systemMode != AppSpecCPP::SystemMode::Auto) {
            effTemperature = UtilityHelper::toFahrenheit(schedule()->effectiveTemperature(systemSetup()->systemMode));

        } else {

            // Use auto mode values by default and change later in schedule if needed.
            double minReqTemp = mAutoMinReqTempF;
            double maxReqTemp = mAutoMaxReqTempF;

            // Use schedule and use the auto mode logics.
            if (schedule()) {
                minReqTemp = UtilityHelper::toFahrenheit(schedule()->minimumTemperature);
                maxReqTemp = UtilityHelper::toFahrenheit(schedule()->maximumTemperature);
            }

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

    return effTemperature;
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

double SchemeDataProvider::dualFuelThreshodF() const
{
    return UtilityHelper::toFahrenheit(mSystemSetup->dualFuelThreshod);
}

int SchemeDataProvider::heatPumpStage() const
{
    return mSystemSetup->coolStage;
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
