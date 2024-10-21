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
}


void SystemSetup::updateMode(AppSpecCPP::SystemMode mode)
{
    if (mode != systemMode) {
        systemMode = mode;
        emit systemModeChanged();
    }
}

void SystemSetup::setupPerfTest(AppSpecCPP::SystemMode mode)
{
    qDebug() <<"setupPerfTest: " <<mode <<", current mode: " <<systemMode;
    mVacationBeforePerfTest = isVacation;
    isVacation = false;
    mModeBeforePerfTest = systemMode;
    systemMode = mode;
    isPerfTestRunning(true);
}

void SystemSetup::revertPerfTest()
{
    if (!isPerfTestRunning()) return;

    qDebug() << "revertPerfTest: " << mModeBeforePerfTest <<", current mode: " << systemMode;
    updateMode(mModeBeforePerfTest);
    mModeBeforePerfTest = AppSpecCPP::Off;
    isPerfTestRunning(false);

    if (mVacationBeforePerfTest) {
        isVacation = mVacationBeforePerfTest;
        mVacationBeforePerfTest = false;
        emit isVacationChanged();
    }
}
