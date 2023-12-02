#include "SystemSetup.h"

SystemSetup::SystemSetup(QSObjectCpp *parent)
    : QSObjectCpp{parent}
{
    // defaults
    systemType = AppSpecCPP::SystemType::Conventional;

    traditionalHeatStage = 1;
    traditionalCoolStage = 1;

    heatPumpStage = 1;

    heatPumpOBState = 0;

    coolStage = 1;
    heatStage = 1;

    systemRunDelay = 1;

    systemMode = AppSpecCPP::SystemMode::Auto;

    heatPumpEmergency = false;
}
