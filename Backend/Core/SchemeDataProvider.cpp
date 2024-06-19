#include "SchemeDataProvider.h"

#include "LogHelper.h"
#include "UtilityHelper.h"

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
