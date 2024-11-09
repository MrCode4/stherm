#include "SystemSetup.h"

SystemSetup::SystemSetup(QSObjectCpp *parent)
    : QSObjectCpp{parent}
{
    // defaults
    systemType = AppSpecCPP::SystemType::SysTUnknown;

    heatPumpOBState = 0;

    coolStage = 1;
    heatStage = 1;

    systemRunDelay = 5;

    systemMode = AppSpecCPP::SystemMode::Off;

    systemAccessories = new SystemAccessories(this);
    isVacation = false;

    _mIsSystemShutoff = false;

    dualFuelThreshod = 1.666667; // 35 Fahrenheit

    //! Initialize emergency properties
    emergencyMinimumTime = 2;
    emergencyControlType = AppSpecCPP::ECTManually;
    emergencyTemperatureDiffrence = 1.6;

}
