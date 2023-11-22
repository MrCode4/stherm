#pragma once

#include "UtilityHelper.h"


/*! ***********************************************************************************************
 * Relay: Check and update relay factors.
 * ************************************************************************************************/

class Relay
{
public:
    Relay();

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

private:
    void backlight();

private:
    STHERM::Relay mRelay;

    STHERM::SystemMode before_state;
    STHERM::SystemMode  current_state;

    int current_stage;

    QString ob_state;
};

