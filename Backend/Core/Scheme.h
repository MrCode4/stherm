#pragma once

#include <QThread>
#include <QVariantMap>

#include "Core/Relay.h"
#include "Device/SystemSetup.h"
#include "DeviceAPI.h"
#include "ScheduleCPP.h"
#include "UtilityHelper.h"
#include "AppSpecCPP.h"
#include "include/timing.h"

/*! ***********************************************************************************************
 * THis class manage Vacation data.
 * todo: Add another properties.
 * ************************************************************************************************/

enum ChangeType {
    CurrentTemperature = 0,
    SetTemperature,
    Mode,

};

class Scheme : public QThread
{
    Q_OBJECT

public:
    explicit Scheme(DeviceAPI *deviceAPI, QObject *parent = nullptr);

    void stop();
    ~Scheme();

    AppSpecCPP::SystemMode getCurrentSysMode() const;
    void setCurrentSysMode(AppSpecCPP::SystemMode newSysMode);


    void setMainData(QVariantMap mainData);

    //! Update Humidifier Id
    void setHumidifierId(const int &humidifierId);


    //! Setter and getter for set point humidity.
    double setPointHimidity() const;
    void setSetPointHimidity(double newSetPointHimidity);

    //! Setter and getter for set current humidity.
    //! Update from setMainData.
    double currentHumidity() const;
    void setCurrentHumidity(double newCurrentHumidity);

    void setFanWorkPerHour(int newFanWPH);

    void setSystemSetup(SystemSetup* systemSetup);

    //! Set requested Temperature
    void setSetPointTemperature(double newSetPointTemperature);

    //! Set requested Humidity
    void setRequestedHumidity(double newHumidity);

    //! Restart the worker thread
    void restartWork();

    void setVacation(const STHERM::Vacation &newVacation);

    void setSchedule(ScheduleCPP *newSchedule);

signals:
    //! Change backlight with the mode
    //!changeBacklight() without any parameters resets the backlight to its original value
    void changeBacklight(QVariantList colorData = QVariantList(),
                         QVariantList colorDataAfter = QVariantList());

    //! Send relay to DeviceIOController and update relays into ti board.
    void updateRelays(STHERM::RelayConfigs);

    void alert();

    void currentTemperatureChanged();
    void setTemperatureChanged();
    void stopWorkRequested();

protected:
    void run() override;

private slots:

private:
    void updateParameters();
    void resetDelays();

    void AutoModeLoop();
    void CoolingLoop();
    void HeatingLoop();
    void VacationLoop();
    void EmergencyLoop();
    void OffLoop();

    void internalCoolingLoopStage1(bool pumpHeat);
    bool internalCoolingLoopStage2();

    void internalHeatingLoopStage1();
    bool internalHeatingLoopStage2();
    bool internalHeatingLoopStage3();

    void internalPumpHeatingLoopStage1();
    bool internalPumpHeatingLoopStage2();
    void EmergencyHeating();
    void sendAlertIfNeeded();

    //! Send relays into ti
    void sendRelays();

    //! Update vacation mode
    void updateVacationState();

    //! To monitor data change: current temperature, set temperature, mode
    int waitLoop(int timeout = 10000);

    //! Update humidifire and dehumidifire after changes: mode, set point humidity,
    //! current humidity, and humidifier Id
    void updateHumifiresState();

    //! Find the effective temperature to run the system with founded temperature
    //! return the tempereture as Fahrenheit
    double effectiveTemperature();

private:
    /* Attributes
     * ****************************************************************************************/
    DeviceAPI *mDeviceAPI;

    QVariantMap _mainData;

    AppSpecCPP::SystemMode mCurrentSysMode;

    AppSpecCPP::SystemMode mRealSysMode;

    ScheduleCPP* mSchedule;

    struct STHERM::Vacation mVacation;

    SystemSetup *mSystemSetup = nullptr;

    NUVE::Timing* mTiming;
    Relay*  mRelay;

    int mHumidifierId;

    //! Humidity parameters
    double mCurrentHumidity;
    double mSetPointHimidity;

    //! Temperature parameters
    double mCurrentTemperature;
    double mSetPointTemperature;

    // Fan work per hour (minutes per hour) Range: 0 - 60
    int mFanWPH;

    bool stopWork;
    bool isVacation;
};
