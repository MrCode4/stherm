#include "SystemSetup.h"

SystemSetup::SystemSetup(QSObjectCpp *parent)
    : QSObjectCpp{parent}
{
    // defaults
    systemType = AppSpecCPP::SystemType::Conventional;

    heatPumpOBState = 0;

    coolStage = 1;
    heatStage = 1;

    systemRunDelay = 1;

    systemMode = AppSpecCPP::SystemMode::Auto;

    heatPumpEmergency = false;
    isVacation = false;
}
