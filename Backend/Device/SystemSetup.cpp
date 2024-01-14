#include "SystemSetup.h"

SystemSetup::SystemSetup(QSObjectCpp *parent)
    : QSObjectCpp{parent}
{
    // defaults
    systemType = AppSpecCPP::SystemType::Conventional;

    heatPumpOBState = 0;

    coolStage = 1;
    heatStage = 1;

    systemRunDelay = 5;

    systemMode = AppSpecCPP::SystemMode::Off;

    systemAccessories = new SystemAccessories(this);
    isVacation = false;
}
