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
    isAUXAuto = true;
    dualFuelManualHeating = AppSpecCPP::DFMOff;
    dualFuelHeatingModeDefault = AppSpecCPP::DFMHeatPump;

    //! Initialize emergency properties
    emergencyMinimumTime = AppSpecCPP::defaultEmergencyMinimumTime();
    emergencyControlType = AppSpecCPP::ECTManually;
    emergencyTemperatureDifference = AppSpecCPP::defaultEmergencyTemperatureDifferenceC();
    emergencyTemperatureDiffrence = AppSpecCPP::defaultEmergencyTemperatureDifferenceC();

    auxiliaryHeating = true;
    useAuxiliaryParallelHeatPump = true;
    driveAux1AndETogether = true;
    enableEmergencyModeForAuxStages = true;


    connect(this, &SystemSetup::emergencyTemperatureDiffrenceChanged, this, [this] () {
        // we will not apply if the other already has change
        if (emergencyTemperatureDifference == AppSpecCPP::defaultEmergencyTemperatureDifferenceC())
            emergencyTemperatureDifference = emergencyTemperatureDiffrence;
    });
}
