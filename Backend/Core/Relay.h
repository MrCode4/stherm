#pragma once

#include <QObject>

#include "UtilityHelper.h"
#include "AppSpecCPP.h"


/*! ***********************************************************************************************
 * Relay: Check and update relay factors.
 * ************************************************************************************************/

class Relay : public QObject
{
    Q_OBJECT

public:

    static Relay* instance();

    int getCoolingMaxStage();
    int getHeatingMaxStage();

    /**
     * Start or restart timer for main state and stage
     */
    void startTempTimer(AppSpecCPP::SystemMode current_state);

    void updateStates();
    void setAllOff();

    bool heatingStage0();
    bool heatingStage1(bool heatpump = false);
    bool heatingStage3(bool heatpump = false);
    bool heatingStage2(bool heatpump = false);

    bool coolingStage0();
    bool coolingStage1();
    bool coolingStage2();
    bool coolingStage3();

    bool emergencyHeating1();
    bool emergencyHeating2();

    bool fanWorkTime(int fanWPH, int interval);

    STHERM::RelayConfigs relays();

    bool turnOffEmergencyHeating();

    AppSpecCPP::SystemMode getOb_state() const;
    void setOb_state(AppSpecCPP::SystemMode newOb_state);

    AppSpecCPP::SystemMode currentState() const;

    //! Update Humidifier state
    void setHumidifierState(const bool on);

    //! Update Dehumidifier state
    void setDehumidifierState(const bool on);

    AppSpecCPP::SystemMode getOb_on_state() const;
    void setOb_on_state(const AppSpecCPP::SystemMode &newOb_on_state);

private:
    explicit Relay();

    void backlight();
    
    void fanOn();
    void fanOFF();

    void updateOB();

private:
    static Relay* mInstance;

    STHERM::RelayConfigs mRelay;

    AppSpecCPP::SystemMode before_state;
    AppSpecCPP::SystemMode current_state;
    AppSpecCPP::SystemMode ob_state; // can be Cooling or Heating
    AppSpecCPP::SystemMode ob_on_state; // can be Cooling or Heating

    int current_stage;

};

