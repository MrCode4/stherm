#include "SystemSetup.h"

#include <QTimer>

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

    // TODO: remove
    dualFuelThreshod = 1.666667; // 35 Fahrenheit

    dualFuelThreshold = 1.666667; // 35 Fahrenheit
    isAUXAuto = true;
    dualFuelManualHeating = AppSpecCPP::DFMOff;
    dualFuelHeatingModeDefault = AppSpecCPP::DFMHeatPump;

    //! Initialize emergency properties
    emergencyMinimumTime = AppSpecCPP::defaultEmergencyMinimumTime();

    auxiliaryHeating = true;
    useAuxiliaryParallelHeatPump = true;
    driveAux1AndETogether = true;
    driveAuxAsEmergency = true;
    runFanWithAuxiliary = true;

    // TODO: remove
    // To call after model loaded completely.
    QTimer::singleShot(50, this, [this](){
        dualFuelThreshold = dualFuelThreshod;
        _connectDualFuelThreshodChanged();
    });
}

void SystemSetup::_connectDualFuelThreshodChanged() {
    connect(this, &SystemSetup::dualFuelThresholdChanged, this, [this]() {
        dualFuelThreshod = dualFuelThreshold;
    });
}
