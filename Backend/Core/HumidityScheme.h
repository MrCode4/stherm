#pragma once

#include <QObject>
#include <QThread>

#include "DeviceAPI.h"
#include "Relay.h"
#include "ScheduleCPP.h"
#include "SystemSetup.h"

/*! ***********************************************************************************************
 * This class controls the humidity.
 * ************************************************************************************************/

class HumidityScheme : public QThread
{
    Q_OBJECT

public:
    explicit HumidityScheme(DeviceAPI *deviceAPI, QObject *parent = nullptr);

    void setSystemSetup(SystemSetup *systemSetup);

protected:
    void run() override;

signals:

private:
    //! Restart the worker thread
    void restartWork();

    //! Vacation loop
    void VacationLoop();

    //! Auto mode loop used in schedule
    void AutoModeLoop();

    void setVacation(const STHERM::Vacation &newVacation);

    //! Set schedule
    void setSchedule(ScheduleCPP *newSchedule);

private:
    Relay*  mRelay;
    DeviceAPI *mDeviceAPI;

    SystemSetup *mSystemSetup = nullptr;
    ScheduleCPP* mSchedule = nullptr;

    AppSpecCPP::AccessoriesType     mAccessoriesType;
    AppSpecCPP::AccessoriesWireType mAccessoriesWireType;

    //! Vacation properites (Percentage)
    double mVacationMinimumHumidity;
    double mVacationMaximumHumidity;

    bool stopWork;
};

