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

void Scheme::updateRealState(const struct STHERM::Vacation &vacation, const double &setTemperature, const double &currentTemperature, const double &currentHumidity)
{
    if (mRealSysMode == STHERM::SystemMode::Cooling) {
       mRealSysMode = updateNormalState(setTemperature, currentTemperature, currentHumidity);

    } else if (mRealSysMode == STHERM::SystemMode::Vacation) {
       mRealSysMode = updateVacationState(vacation, setTemperature, currentTemperature, currentHumidity);

    }

    if (mRealSysMode == STHERM::SystemMode::Heating) {
       if (currentTemperature < ET) {
           mRealSysMode = STHERM::SystemMode::Emergency;
//           set_stage = 1;
           if (currentTemperature < ET - ET_STAGE2) {
//               set_stage = 2;
           }
       }
    }

    if (mRealSysMode != mCurentSysMode) { // mode changes
//       current_stage = 0;
    }


}

STHERM::SystemMode Scheme::updateVacationState(const struct STHERM::Vacation &vacation,
                                          const double &setTemperature,
                                          const double &currentTemperature,
                                          const double &currentHumidity)
{
    STHERM::SystemMode realSetMode;
    if (mCurentSysMode == STHERM::SystemMode::Cooling) {
        if (currentTemperature > setTemperature - STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Cooling;
        } else if (currentTemperature > setTemperature - STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Heating;
        }
    } else if (mCurentSysMode == STHERM::SystemMode::Heating) {
        if (currentTemperature < setTemperature + STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Heating;
        } else if (currentTemperature < setTemperature + STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Cooling;
        }
    } else { // OFF
        if (currentTemperature < setTemperature - STAGE1_ON_RANGE) {
            realSetMode = STHERM::SystemMode::Heating;
        } else if (currentTemperature > setTemperature + STAGE1_ON_RANGE) {
            realSetMode = STHERM::SystemMode::Cooling;
        }
    }

    if (vacation.minimumTemperature > currentTemperature) {
        realSetMode = STHERM::SystemMode::Heating;
        //        range = temperature - $current_state['min_temp'];
    } else if (vacation.maximumTemperature < currentTemperature) {
        realSetMode = STHERM::SystemMode::Cooling;
        //        range = temperature - current_state['max_temp'];
    }

    if (mHumidifierId == 1) {
        if (currentHumidity < vacation.minimumHumidity) {
            setHumidifierState(true);
        } else {
            setHumidifierState(false);
        }
    } else if (mHumidifierId == 2) {
        if (currentHumidity > vacation.maximumHumidity) {
            setDehumidifierState(true);
            if (currentHumidity <= vacation.minimumHumidity) {
                setDehumidifierState(false);
            }
        }
    }

    return realSetMode;
}

STHERM::SystemMode Scheme::updateNormalState(const double &setTemperature, const double &currentTemperature, const double &currentHumidity)
{
    STHERM::SystemMode realSetMode;

    if (mCurentSysMode == STHERM::SystemMode::Cooling) {
        if (currentTemperature > setTemperature - STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Cooling;
        } else if (currentTemperature > setTemperature - STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Heating;
        }
    } else if (mCurentSysMode == STHERM::SystemMode::Heating) {
        if (currentTemperature < setTemperature + STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Heating;
        } else if (currentTemperature < setTemperature + STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Cooling;
        }
    } else { // OFF
        if (currentTemperature < setTemperature - STAGE1_ON_RANGE) {
            realSetMode = STHERM::SystemMode::Heating;
        } else if (currentTemperature > setTemperature + STAGE1_ON_RANGE) {
            realSetMode = STHERM::SystemMode::Cooling;
        }
    }

    return realSetMode;
}

void Scheme::startHumidifierWork(int humidifier, QString device_state, int humidity, int current_humidity, int sth, int stl)
{

}

void Scheme::setCurrentState(const int &humidifierId)
{
    mHumidifierId = humidifierId;

}

STHERM::SystemMode Scheme::getCurrentSysMode() const
{
    return mCurentSysMode;
}

void Scheme::setCurrentSysMode(STHERM::SystemMode newSysMode)
{
    if (mCurentSysMode == newSysMode)
        return;

    mCurentSysMode = newSysMode;
}

void Scheme::setHumidifierState(bool on) {

}

void Scheme::setDehumidifierState(bool on)
{

}

STHERM::SystemMode Scheme::realSysMode() const
{
    return mRealSysMode;
}

void Scheme::setRealSysMode(STHERM::SystemMode newRealSysMode)
{
    mRealSysMode = newRealSysMode;
}
