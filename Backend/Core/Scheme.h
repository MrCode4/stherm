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

    //! Set requested Temperature
    void setSetPointTemperature(double newSetPointTemperature);

    //! Restart the worker thread
    void restartWork() override;

    void setVacation() override;

    void moveToUpdatingMode();

    AppSpecCPP::FanMode fanMode() const;

    //! Set Auto temperature ranges
    void setAutoMinReqTemp(const double &min);
    void setAutoMaxReqTemp(const double &max);

    void runSystemDelay(AppSpecCPP::SystemMode mode);

signals:
    //! Change backlight with the mode
    //!changeBacklight() without any parameters resets the backlight to its original value
    void changeBacklight(QVariantList colorData = QVariantList(),
                         QVariantList colorDataAfter = QVariantList());

    void alert();

    void setTemperatureChanged();

    void currentSystemModeChanged(AppSpecCPP::SystemMode obState);

    //! Start system delay timer in ui to show in home page
    //! delay: miliseconds
    void startSystemDelayCountdown(AppSpecCPP::SystemMode mode, int delay);

    //! stop system delay timer
    void stopSystemDelayCountdown();

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

    int mHumidifierId;

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

    bool isVacation;
    bool mRestarting;
};
