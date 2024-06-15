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

    void setVacation(const STHERM::Vacation &newVacation);

    //! Set schedule
    void setSchedule(ScheduleCPP *newSchedule);

    void setRequestedHumidity(const double& setPointHumidity);

protected:
    void run() override;

signals:

private:
    //! Restart the worker thread
    void restartWork();

    //! Vacation loop
    void VacationLoop();

    //! Normal loop
    void normalLoop();

    //! Auto mode loop used in schedule
    void AutoModeLoop();

    //! Update relays but not sent to device.
    //! None sets the humidity wirings to off.
    void updateRelays(AppSpecCPP::AccessoriesWireType accessoriesWireType = AppSpecCPP::None);

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

    double mSetPointHumidity;

    //! Humidity parameters (Percentage)
    double mCurrentHumidity = 30;

    bool stopWork;
};

