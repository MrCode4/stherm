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
    
    void setFan(AppSpecCPP::FanMode fanMode, int newFanWPH);

    void setSystemSetup(SystemSetup* systemSetup);

    //! Set requested Temperature
    void setSetPointTemperature(double newSetPointTemperature);

    //! Set requested Humidity
    void setRequestedHumidity(double newHumidity);

    //! Restart the worker thread
    void restartWork();

    void setVacation(const STHERM::Vacation &newVacation);

    void setSchedule(ScheduleCPP *newSchedule);

    void moveToUpdatingMode();

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

    void fanWorkChanged(bool fanState);

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

    void updateOBState(AppSpecCPP::SystemMode newOb_state);

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
    //! use low values for timeout in exit cases as it might had abrupt changes previously
    int waitLoop(int timeout = 10000);

    //! Update humidifire and dehumidifire after changes: mode, set point humidity,
    //! current humidity, and humidifier Id
    void updateHumifiresState();

    //! Find the effective temperature to run the system with found temperature
    //! based on schedule or vacation settings
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

    //! Temperature parameters
    double mCurrentTemperature;
    double mSetPointTemperature;

    //! Fan work per hour (minutes per hour) Range: 0 - 60
    int mFanWPH;
    AppSpecCPP::FanMode mFanMode;

    bool stopWork;
    bool isVacation;
    bool mRestarting;

    STHERM::RelayConfigs lastConfigs;

    bool debugMode = true;

    void fanWork(bool isOn);
};
