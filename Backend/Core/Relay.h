#pragma once

#include <QObject>

#include "UtilityHelper.h"


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
    void startTempTimer(STHERM::SystemMode current_state);

    void updateStates();
    void setAllOff();

    bool heatingStage0();
    bool heatingStage1();
    bool heatingStage3();
    bool heatingStage2();

    bool coolingStage0();
    bool coolingStage1();
    bool coolingStage2();
    bool coolingStage3();

    bool emergencyHeating1();
    bool emergencyHeating2();

    bool fanWorkTime();

    STHERM::RelayConfigs relays();

    bool turnOffEmergencyHeating();

    STHERM::SystemMode getOb_state() const;
    void setOb_state(STHERM::SystemMode newOb_state);

    STHERM::SystemMode currentState() const;

    //! Update Humidifier state
    void setHumidifierState(const bool on);

    //! Update Dehumidifier state
    void setDehumidifierState(const bool on);

private:
    explicit Relay();

    void backlight();

private:
    static Relay* mInstance;

    STHERM::RelayConfigs mRelay;

    STHERM::SystemMode before_state;
    STHERM::SystemMode  current_state;
    STHERM::SystemMode ob_state; // can be Cooling or Heating

    int current_stage;

};

