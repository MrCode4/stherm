#pragma once

#include <QVariantMap>
#include <QTimer>

#include "BaseScheme.h"

/*! ***********************************************************************************************
 * This class manage temperature loops.
 * ************************************************************************************************/

class Scheme : public BaseScheme
{
    Q_OBJECT

public:
    explicit Scheme(DeviceAPI *deviceAPI, QSharedPointer<SchemeDataProvider> schemeDataProvider, QObject *parent = nullptr);

    void stop();
    ~Scheme();

    AppSpecCPP::SystemMode getCurrentSysMode() const;
    void setCurrentSysMode(AppSpecCPP::SystemMode newSysMode);

    //! Setter and getter for set point humidity.
    double setPointHimidity() const;
    void setSetPointHimidity(double newSetPointHimidity);

    void setFan(AppSpecCPP::FanMode fanMode, int newFanWPH);

    void setSystemSetup() override;

    //! Restart the worker thread
    void restartWork(bool forceStart = false) override;

    void setVacation() override;

    void moveToUpdatingMode();

    AppSpecCPP::FanMode fanMode() const;

    void runSystemDelay(AppSpecCPP::SystemMode mode);

    AppSpecCPP::SystemType activeSystemTypeHeating();

    void switchDFHActiveSysType(AppSpecCPP::SystemType to);

signals:
    //! Change backlight with the mode
    //!changeBacklight() without any parameters resets the backlight to its original value
    void changeBacklight(QVariantList colorData = QVariantList(),
                         QVariantList colorDataAfter = QVariantList());

    void alert();

    void currentSystemModeChanged(AppSpecCPP::SystemMode obState, int currentHeatingStage, int currentCoolingStage);

    //! Start system delay timer in ui to show in home page
    //! delay: miliseconds
    void startSystemDelayCountdown(AppSpecCPP::SystemMode mode, int delay);

    //! stop system delay timer
    void stopSystemDelayCountdown();

    //! Active system mode changed due to dual fuel heating
    void dfhSystemTypeChanged(AppSpecCPP::SystemType activeSystemType);

    void actualModeStarted(AppSpecCPP::SystemMode mode);

    //! To block mode change in UI
    //! The system mode change will be unblock after `miliSecs` mili-seconds.
    void manualEmergencyModeUnblockedAfter(int miliSecs);

protected:
    void run() override;

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

    //! Users manually activate emergency heat., meaning Emergency heating will only be active when the system mode is set to Emergency or
    //!  emergency heating will be triggered by the Defrost Controller Board (if equipped) or based on system needs.
    void emergencyHeatingLoop();
    void sendAlertIfNeeded();

    //! Send relays into ti
    void sendRelays(bool forceSend = false);

    //! Update vacation mode
    void updateVacationState();

    //! To monitor data change: current temperature, set temperature, mode
    //! use low values for timeout in exit cases as it might had abrupt changes previously
    int waitLoop(int timeout = 10000, AppSpecCPP::ChangeTypes overrideModes = AppSpecCPP::ChangeType::ctAll) override;

    //! Find the effective temperature to run the system with found temperature
    //! based on schedule or vacation settings and systemMode (auto)
    //! In auto mode set the effective temperature to the current temperature to shutdown the system
    //! when mCurrentTemperature is in aut of auto range
    //! return the temperature as Fahrenheit
    double effectiveTemperature();

    //! find the currentTemperature based on source sensor or any overriding rule!
    double effectiveCurrentTemperature();

    void fanWork(bool isOn);

    void checkForRestartDualFuel();

    void manualEmergencyHeating();

private:
    /* Attributes
     * ****************************************************************************************/

    AppSpecCPP::SystemMode mCurrentSysMode;

    AppSpecCPP::SystemMode mRealSysMode;

    NUVE::Timing* mTiming;

    QTimer mUpdatingTimer;

    //! Fan hours loop
    QTimer mFanHourTimer;

    //! Fan work (minutes) per hour loop
    QTimer mFanWPHTimer;

    //! to log vital informations
    QTimer mLogTimer;

    //! Vacation properites (Fahrenheit)
    double mVacationMinimumTemperature;
    double mVacationMaximumTemperature;

    //! Fan work per hour (minutes per hour) Range: 0 - 60
    int mFanWPH;
    AppSpecCPP::FanMode mFanMode;

    bool isVacation;
    bool mRestarting;

    //! Use for minimum run time of emergency heating
    QElapsedTimer mTEONTimer;

    //! Switch active system type in the dual fuel heating to ...
    //! Used in internet connection troubleshooting
    AppSpecCPP::SystemType mSwitchDFHActiveSysTypeTo;
    //! Used in deciding if we need to restart on some changes in dual fuel
    AppSpecCPP::SystemType mActiveSysTypeHeating;
};
