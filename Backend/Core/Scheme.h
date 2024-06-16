#pragma once

#include <QThread>
#include <QVariantMap>
#include <QTimer>

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
    
    void setFan(AppSpecCPP::FanMode fanMode, int newFanWPH);

    void setSystemSetup(SystemSetup* systemSetup);

    //! Set requested Temperature
    void setSetPointTemperature(double newSetPointTemperature);

    //! Restart the worker thread
    void restartWork();

    void setVacation(const STHERM::Vacation &newVacation);

    void setSchedule(ScheduleCPP *newSchedule);

    void moveToUpdatingMode();

    AppSpecCPP::FanMode fanMode() const;

    //! Set Auto temperature ranges
    void setAutoMinReqTemp(const double &min);
    void setAutoMaxReqTemp(const double &max);

    void setCanSendRelays(const bool& csr);

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

    //! Emit when we need exit from wait loop
    void stopWorkRequested();

    void fanWorkChanged(bool fanState);

    void currentSystemModeChanged(AppSpecCPP::SystemMode obState);

    void sendRelayIsRunning(const bool& isRunning);
    void canSendRelay();

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

    bool updateOBState(AppSpecCPP::SystemMode newOb_state);

    void internalCoolingLoopStage1();
    bool internalCoolingLoopStage2();

    void internalHeatingLoopStage1();
    bool internalHeatingLoopStage2();
    bool internalHeatingLoopStage3();

    void internalPumpHeatingLoopStage1();
    bool internalPumpHeatingLoopStage2();
    void EmergencyHeating();
    void sendAlertIfNeeded();

    //! Send relays into ti
    void sendRelays(bool forceSend = false);

    //! Update vacation mode
    void updateVacationState();

    //! To monitor data change: current temperature, set temperature, mode
    //! use low values for timeout in exit cases as it might had abrupt changes previously
    int waitLoop(int timeout = 10000, AppSpecCPP::ChangeTypes overrideModes = AppSpecCPP::ChangeType::ctAll);

    //! Update humidifire and dehumidifire after changes: mode, set point humidity,
    //! current humidity, and humidifier Id
    void updateHumifiresState();

    //! Find the effective temperature to run the system with found temperature
    //! based on schedule or vacation settings and systemMode (auto)
    //! In auto mode set the effective temperature to the current temperature to shutdown the system
    //! when mCurrentTemperature is in aut of auto range
    //! return the temperature as Fahrenheit
    double effectiveTemperature();

    //! find the currentTemperature based on source sensor or any overriding rule!
    double effectiveCurrentTemperature();


    //! Find the effective humidity to run the system with found humidity
    //! based on schedule or vacation settings
    double effectiveHumidity();

    //! find the currentHumidity based on source sensor or any overriding rule!
    double effectiveCurrentHumidity();

private:
    /* Attributes
     * ****************************************************************************************/
    DeviceAPI *mDeviceAPI;

    QVariantMap _mainData;

    AppSpecCPP::SystemMode mCurrentSysMode;

    AppSpecCPP::SystemMode mRealSysMode;

    ScheduleCPP* mSchedule = nullptr;

    struct STHERM::Vacation mVacation;

    SystemSetup *mSystemSetup = nullptr;

    NUVE::Timing* mTiming;
    Relay*  mRelay;

    QTimer mUpdatingTimer;

    //! Fan hours loop
    QTimer mFanHourTimer;

    //! Fan work (minutes) per hour loop
    QTimer mFanWPHTimer;

    //! to log vital informations
    QTimer mLogTimer;

    int mHumidifierId;

    //! Humidity parameters
    double mCurrentHumidity;
    double mSetPointHimidity;

    //! Temperature parameters (Fahrenheit)
    double mCurrentTemperature = 20 * 1.8 + 32;

    //! Fahrenheit
    double mSetPointTemperature;

    //! Auto mode properites (Fahrenheit)
    double mAutoMinReqTemp;
    double mAutoMaxReqTemp;

    //! Vacation properites (Fahrenheit)
    double mVacationMinimumTemperature;
    double mVacationMaximumTemperature;

    //! Fan work per hour (minutes per hour) Range: 0 - 60
    int mFanWPH;
    AppSpecCPP::FanMode mFanMode;

    bool stopWork;
    bool isVacation;
    bool mRestarting;

    STHERM::RelayConfigs lastConfigs;

    bool debugMode = true;

    bool mCanSendRelay;

    void fanWork(bool isOn);
};
