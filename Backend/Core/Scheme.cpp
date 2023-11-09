#include "Scheme.h"

const double ET                      = 40;                 // 4.5 C
const double ET_STAGE2               = 3.5;
const double HPT                     = 60;                // 15 C
const double HUM_MAX                 = 70;
const double HUM_MIN                 = 20;
const double HUM_STEP                = 10;
const double STAGE0_RANGE            = 0;
const double STAGE1_ON_RANGE         = 1.9;
const double STAGE2_ON_RANGE         = 2.9;
const double STAGE3_ON_RANGE         = 5.9;
const double STAGE1_OFF_RANGE        = 1;
const double STAGE2_OFF_RANGE        = 1.9;
const double STAGE3_OFF_RANGE        = 4.9;
const double ALERT_TIME              = 120;
const double CHANGE_STAGE_TIME       = 40;
const double CHANGE_STAGE_TIME_WO_OB = 10;
const double S2OFF_TIME              = 2;

Scheme::Scheme(QObject *parent) :
    QObject (parent)
{

}

STHERM::SystemMode Scheme::updateVacation(const struct STHERM::Vacation &vacation,
                                          const double &setTemperature,
                                          const double &temperature)
{
    STHERM::SystemMode realSetMode;
    if (sysMode == STHERM::SystemMode::Cooling) {
        if (temperature > setTemperature - STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Cooling;
        } else if (temperature > setTemperature - STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Heating;
        }
    } else if (sysMode == STHERM::SystemMode::Heating) {
        if (temperature < setTemperature + STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Heating;
        } else if (temperature < setTemperature + STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Cooling;
        }
    } else { // OFF
        if (temperature < setTemperature - STAGE1_ON_RANGE) {
            realSetMode = STHERM::SystemMode::Heating;
        } else if (temperature > setTemperature + STAGE1_ON_RANGE) {
            realSetMode = STHERM::SystemMode::Cooling;
        }
    }

    if (vacation.minimumTemperature > temperature) {
        realSetMode = STHERM::SystemMode::Heating;
        //        range = temperature - $current_state['min_temp'];
    } else if (vacation.maximumTemperature < temperature) {
        realSetMode = STHERM::SystemMode::Cooling;
        //        range = temperature - current_state['max_temp'];
    }

    return realSetMode;
}

STHERM::SystemMode Scheme::getSysMode() const
{
    return sysMode;
}

void Scheme::setSysMode(STHERM::SystemMode newSysMode)
{
    if (sysMode == newSysMode)
        return;

    sysMode = newSysMode;
}
