#pragma once

#include <QThread>
#include <QVariantMap>

#include "Core/Relay.h"
#include "Device/SystemSetup.h"
#include "DeviceAPI.h"
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

    ~Scheme();

    void updateRealState(const struct STHERM::Vacation &vacation,
                         const double &setTemperature,
                         const double &currentTemperature,
                         const double &currentHumidity);

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

signals:
    //! Change backlight with the mode
    //!changeBacklight() without any parameters resets the backlight to its original value
    void changeBacklight(QVariantList colorData = QVariantList(), int secs = 5);

    //! Send relay to DeviceIOController and update relays into ti board.
    void updateRelays(STHERM::RelayConfigs);

    void alert();

    void currentTemperatureChanged();
    void setTemperatureChanged();

protected:
    virtual void run();

private:
    void startWork();

    //! Send relays into ti
    void sendRelays();

    //! Update vacation mode
    void updateVacationState();

    AppSpecCPP::SystemMode updateNormalState(const double &setTemperature,
                                         const double &currentTemperature,
                                         const double &currentHumidity);

    //! Cooling and heating roles.
    void coolingHeatPumpRole1(bool needToWait = true);
    void coolingHeatPumpRole2();
    void heatingHeatPumpRole1();
    void heatingHeatPumpRole2(bool needToWait = true);
    void heatingHeatPumpRole3();
    void heatingConventionalRole1(bool needToWait = true);
    void heatingConventionalRole2();
    void heatingConventionalRole3();

    //! Emergency heating roles
    void heatingEmergencyHeatPumpRole1();
    void heatingEmergencyHeatPumpRole2();
    void heatingEmergencyHeatPumpRole3();

    //! To monitor data change: current temperature, set temperature, mode
    int waitLoop();

    //! Update humidifire and dehumidifire after changes: mode, set point humidity,
    //! current humidity, and humidifier Id
    void updateHumifiresState();

private:
    /* Attributes
     * ****************************************************************************************/
    DeviceAPI *mDeviceAPI;

    QVariantMap _mainData;

    AppSpecCPP::SystemMode mCurrentSysMode;

    AppSpecCPP::SystemMode mRealSysMode;

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
};

