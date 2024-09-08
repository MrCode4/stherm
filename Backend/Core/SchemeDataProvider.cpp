#include "SchemeDataProvider.h"

#include "LogHelper.h"
#include "UtilityHelper.h"
#include "Relay.h"

SchemeDataProvider::SchemeDataProvider(QObject *parent)
    : QObject{parent}
{}

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

        // Use auto mode values by default and change later is schedule is defined.
        double minReqTemp = mAutoMinReqTemp;
        double maxReqTemp = mAutoMaxReqTemp;

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

    return effTemperature;
}

void SchemeDataProvider::setAutoMinReqTemp(const double &min)
{
    auto minF = UtilityHelper::toFahrenheit(min);
    if (qAbs(mAutoMinReqTemp - minF) > 0.001)
        mAutoMinReqTemp = minF;
}

double SchemeDataProvider::autoMinReqTemp() const
{
    return mAutoMinReqTemp;
}

void SchemeDataProvider::setAutoMaxReqTemp(const double &max)
{
    auto maxF = UtilityHelper::toFahrenheit(max);
    if (qAbs(mAutoMaxReqTemp - maxF) > 0.001)
        mAutoMaxReqTemp = maxF;
}

double SchemeDataProvider::autoMaxReqTemp() const
{
    return mAutoMaxReqTemp;
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
