#include "SystemSetup.h"

SystemSetup::SystemSetup(QSObjectCpp *parent)
    : QSObjectCpp{parent}
{
    // defaults
    systemType = 0;
    traditionalHeatStage = 1;
    traditionalCoolStage = 1;

    heatPumpStage = 1;

    heatPumpOBState = 0;

    coolStage = 1;
    heatStage = 1;

    heatPumpEmergency = false;
}
