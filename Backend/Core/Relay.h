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

    /**
     * Start or restart timer for main state and stage
     */
    void startTempTimer(AppSpecCPP::SystemMode current_state);

    void setAllOff();

    //! All humidity wiring set to off
    void setAllHumidityWiringsOff();

    bool heatingStage1(bool heatpump = false);
    bool heatingStage3(bool heatpump = false);
    bool heatingStage2(bool heatpump = false);

    bool coolingStage1();
    bool coolingStage2();

    bool emergencyHeating1();
    bool emergencyHeating2();


    STHERM::RelayConfigs relays();
    STHERM::RelayConfigs relaysLast();
    void setRelaysLast(STHERM::RelayConfigs last);

    bool turnOffEmergencyHeating();

    AppSpecCPP::SystemMode getOb_state() const;
    bool setOb_state(AppSpecCPP::SystemMode newOb_state);

    AppSpecCPP::SystemMode currentState() const;

    //! Update Humidifier state
    void setHumidifierState(const bool on);

    //! Update Dehumidifier state
    void setDehumidifierState(const bool on);

    //! Update the Humidity (humidifier/dehumidifier) wiring.
    void updateHumidityWiring(AppSpecCPP::AccessoriesWireType mAccessoriesWireType);

    AppSpecCPP::SystemMode getOb_on_state() const;
    void setOb_on_state(const AppSpecCPP::SystemMode &newOb_on_state);

    void setFanMode(bool on);

    //! This function turn on the w3 as emergency wire.
    bool emergencyHeating3();

    int currentCoolingStage();
    int currentHeatingStage();

    bool auxiliaryHeatingStage1(bool driveAux1AndETogether);

    //! This function toggles the W1 and W2 wires, If we want to keep enable the W1, we should check it again with other functions.
    bool auxiliaryHeatingStage2(bool turnOn = true);

    //! Turn off the heat pump, Y wires.
    bool turnOffHeatPump();

    bool emergencyAuxiliaryHeating(bool useStage3, bool driveAux1AndETogether);

private:
    explicit Relay();

    void backlight();
    
    //! Call when y1 or w1 or mFanOn changed.
    void updateFan();
    void fanOn();
    void fanOFF();

    void updateOB();

private:
    static Relay* mInstance;

    STHERM::RelayConfigs mRelay;
    STHERM::RelayConfigs mRelayLast;

    AppSpecCPP::SystemMode before_state;
    AppSpecCPP::SystemMode current_state;
    AppSpecCPP::SystemMode ob_state; // can be Cooling or Heating or OFF as initial state
    AppSpecCPP::SystemMode ob_on_state; // can be Cooling or Heating

    int current_stage;

    bool mFanOn;

};

