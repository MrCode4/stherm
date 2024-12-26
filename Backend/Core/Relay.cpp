#include "Relay.h"

#include <QDebug>

Relay *Relay::mInstance = nullptr;
Relay *Relay::instance()
{
    if (!mInstance)
        mInstance = new Relay();

    return mInstance;
}


Relay::Relay()
{
    ob_state = AppSpecCPP::SystemMode::Off;
    before_state  = AppSpecCPP::SystemMode::Off;
    current_state = AppSpecCPP::SystemMode::Off;
    current_stage = 0;

    mFanOn = false;
}


//! OBSOLETE
void Relay::updateStates()
{
    if (mRelay.o_b == STHERM::RelayMode::NoWire) { // without OB
        if (mRelay.y1 == STHERM::RelayMode::ON) {
            before_state = AppSpecCPP::SystemMode::Cooling;
            current_stage = 1;
            if (mRelay.y2 == STHERM::RelayMode::ON) {
                current_stage = 2;
            }
            if (mRelay.y3 == STHERM::RelayMode::ON) {
                current_stage = 3;
            }
        } else if (mRelay.w1 == STHERM::RelayMode::ON) {
            before_state = AppSpecCPP::SystemMode::Heating;
            //echo 1;
            current_stage = 1;
            if (mRelay.w2 == STHERM::RelayMode::ON) {
                current_stage = 2;
            }
            if (mRelay.w3 == STHERM::RelayMode::ON) {
                current_stage = 3;
            }
        }
    } else {
        //echo '-----------'.current_state.'|||||||'. before_state.'---------------';
        if (ob_state == AppSpecCPP::SystemMode::Cooling && mRelay.o_b == STHERM::RelayMode::ON) {
            before_state = AppSpecCPP::SystemMode::Cooling;
        } else if(ob_state == AppSpecCPP::SystemMode::Cooling && mRelay.o_b == STHERM::RelayMode::OFF) {
            before_state = AppSpecCPP::SystemMode::Heating;
        } else if (ob_state == AppSpecCPP::SystemMode::Heating && mRelay.o_b == STHERM::RelayMode::ON) {
            before_state = AppSpecCPP::SystemMode::Heating;
        }else if (ob_state == AppSpecCPP::SystemMode::Heating && mRelay.o_b == STHERM::RelayMode::OFF) {
            before_state = AppSpecCPP::SystemMode::Cooling;
        }

        if (mRelay.y1 == STHERM::RelayMode::ON) {
            current_stage = 1;
            if (mRelay.y2 == STHERM::RelayMode::ON) {
                current_stage = 2;
                if (mRelay.y3 == STHERM::RelayMode::ON) {
                    current_stage = 3;
                }
            }
        }
    }
    current_state = before_state;
}

//! OBSOLETE
int Relay::getCoolingMaxStage()
{
    int stage = 0;
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (mRelay.y1 != STHERM::RelayMode::NoWire) {
            stage = 1;
            if (mRelay.y2 != STHERM::RelayMode::NoWire) {
                stage = 2;
            }
        }
    } else {
        if (mRelay.y1 != STHERM::RelayMode::NoWire) {
            stage = 1;
            if (mRelay.y2 != STHERM::RelayMode::NoWire) {
                stage = 2;
                if (mRelay.y3 != STHERM::RelayMode::NoWire) {
                    stage = 3;
                }
            }
        }
    }
    return stage;
}

//! OBSOLETE
int Relay::getHeatingMaxStage()
{
    int stage = 0;
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (mRelay.y1 != STHERM::RelayMode::NoWire) {
            stage = 1;
            if (mRelay.y2 != STHERM::RelayMode::NoWire) {
                stage = 2;
            }
        }
    } else {
        if (mRelay.w1 != STHERM::RelayMode::NoWire) {
            stage = 1;
            if (mRelay.w2 != STHERM::RelayMode::NoWire) {
                stage = 2;
                if (mRelay.w3 != STHERM::RelayMode::NoWire) {
                    stage = 3;
                }
            }
        }
    }
    return stage;
}

bool Relay::coolingStage1()
{
    mRelay.y1 = STHERM::RelayMode::ON;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(AppSpecCPP::SystemMode::Cooling);
    current_state = AppSpecCPP::SystemMode::Cooling;

    return true;
}

void Relay::startTempTimer(AppSpecCPP::SystemMode current_state)
{
    if (current_state != before_state) {
        before_state = current_state;
    }
}


void Relay::backlight()
{
//    //color = self::OFF_RGBM;
//    switch (current_state) {
//    case 'off':
//        //color = self::OFF_RGBM;
//        $color = [0,0,0,0];
//        break;
//    case AppSpecCPP::SystemMode::Cooling:
//        color = self::COOLING_RGBM;
//        break;
//    case 'emergency':
//        color = self::EMERGENCY_RGBM;
//        color_st = self::EMERGENCY_ST_RGBM;
//        break;
//    case AppSpecCPP::SystemMode::Heating:
//        color = self::HEATING_RGBM;
//        break;
    //    }
}

AppSpecCPP::SystemMode Relay::currentState() const
{
    return current_state;
}

void Relay::setHumidifierState(const bool on) {
    mRelay.hum_wiring = on ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
}

void Relay::setDehumidifierState(const bool on) {
    mRelay.hum_wiring = on ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
}

void Relay::updateHumidityWiring(AppSpecCPP::AccessoriesWireType mAccessoriesWireType)
{
    if (mAccessoriesWireType == AppSpecCPP::None) {
        setAllHumidityWiringsOff();
        return;
    }

    mRelay.acc2  = (mAccessoriesWireType == AppSpecCPP::T2PWRD)  ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
    mRelay.acc1p = (mAccessoriesWireType == AppSpecCPP::T1PWRD)  ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
    mRelay.acc1n = (mAccessoriesWireType == AppSpecCPP::T1Short) ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
}

AppSpecCPP::SystemMode Relay::getOb_state() const
{
    return ob_state;
}

bool Relay::setOb_state(AppSpecCPP::SystemMode newOb_state)
{
    if (ob_state != newOb_state) {
        ob_state = newOb_state;
        return true;
    }
    return false;
}

/**
     * Turn set mode Off
     * @return void
     */
void Relay::setAllOff()
{
    mRelay.y1    = STHERM::RelayMode::OFF;
    mRelay.y2    = STHERM::RelayMode::OFF;
    mRelay.w1    = STHERM::RelayMode::OFF;
    mRelay.w2    = STHERM::RelayMode::OFF;
    mRelay.w3    = STHERM::RelayMode::OFF;


    current_state = AppSpecCPP::SystemMode::Off;

}

void Relay::setAllHumidityWiringsOff()
{
    // Humidifire/Dehumidifier relays
    mRelay.acc2  = STHERM::RelayMode::OFF;
    mRelay.acc1p = STHERM::RelayMode::OFF;
    mRelay.acc1n = STHERM::RelayMode::OFF;
}

//! OBSOLETE
bool Relay::heatingStage0()
{
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.y3 = STHERM::RelayMode::OFF;

    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(AppSpecCPP::SystemMode::Heating);
    current_state = AppSpecCPP::SystemMode::Heating;

    return true;
}

//! OBSOLETE
bool Relay::coolingStage0()
{
    mRelay.y1  = STHERM::RelayMode::OFF;
    mRelay.y2  = STHERM::RelayMode::OFF;
    mRelay.y3 = STHERM::RelayMode::OFF;

    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(AppSpecCPP::SystemMode::Cooling);
    current_state = AppSpecCPP::SystemMode::Cooling;

    return true;
}

bool Relay::auxiliaryHeatingStage1(bool driveAux1AndETogether) {
    mRelay.w1 = STHERM::RelayMode::ON;

    if (driveAux1AndETogether)
        mRelay.w3 = STHERM::RelayMode::ON;

    return true;
}

bool Relay::turnOffHeatPump() {
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;

    return true;
}

bool Relay::auxiliaryHeatingStage2(bool turnOn) {
    mRelay.w1 = turnOn ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
    mRelay.w2 = turnOn ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;

    return true;
}

bool Relay::heatingStage1(bool heatpump)
{
    mRelay.y1 = heatpump ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.w1 = heatpump ? STHERM::RelayMode::OFF : STHERM::RelayMode::ON;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(AppSpecCPP::SystemMode::Heating);
    current_state = AppSpecCPP::SystemMode::Heating;

    return true;
}

bool Relay::heatingStage2(bool heatpump)
{
    mRelay.y1 = heatpump ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
    mRelay.y2 = heatpump ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
    mRelay.w1 = heatpump ? STHERM::RelayMode::OFF : STHERM::RelayMode::ON;
    mRelay.w2 = heatpump ? STHERM::RelayMode::OFF : STHERM::RelayMode::ON;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(AppSpecCPP::SystemMode::Heating);
    current_state = AppSpecCPP::SystemMode::Heating;

    return true;
}

bool Relay::coolingStage2()
{
    mRelay.y1 = STHERM::RelayMode::ON;
    mRelay.y2  = STHERM::RelayMode::ON;
    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(AppSpecCPP::SystemMode::Cooling);
    current_state = AppSpecCPP::SystemMode::Cooling;

    return true;
}

bool Relay::heatingStage3(bool heatpump)
{
    // maybe we should assert heatpump as this is not valid when heatpump is true
    if (heatpump) {
        qWarning() << "Heating stage 3 is enabled while using heatpump!!";
    }
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.w1 = STHERM::RelayMode::ON;
    mRelay.w2 = STHERM::RelayMode::ON;
    mRelay.w3 = STHERM::RelayMode::ON;

    startTempTimer(AppSpecCPP::SystemMode::Heating);
    current_state = AppSpecCPP::SystemMode::Heating;

    return true;
}

//! OBSOLETE
bool Relay::coolingStage3()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {

        mRelay.o_b = (ob_state == AppSpecCPP::SystemMode::Cooling) ?
                         STHERM::RelayMode::ON : STHERM::RelayMode::OFF;

    } else {
        mRelay.w1 = STHERM::RelayMode::OFF;
        mRelay.w2 = STHERM::RelayMode::OFF;
        mRelay.w3 = STHERM::RelayMode::OFF;
    }

    mRelay.y1  = STHERM::RelayMode::ON;
    mRelay.y2  = STHERM::RelayMode::ON;
    mRelay.y3  = STHERM::RelayMode::ON;

    startTempTimer(AppSpecCPP::SystemMode::Cooling);
    current_state = AppSpecCPP::SystemMode::Cooling;

    return true;
}

bool Relay::emergencyHeating1()
{
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.w1 = STHERM::RelayMode::ON;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    current_state = AppSpecCPP::SystemMode::Emergency;

    return true;
}

bool Relay::emergencyHeating2()
{
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.w1 = STHERM::RelayMode::ON;
    mRelay.w2 = STHERM::RelayMode::ON;
    mRelay.w3 = STHERM::RelayMode::OFF;

    current_state = AppSpecCPP::SystemMode::Emergency;

    return true;
}

bool Relay::emergencyHeating3()
{
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;

    mRelay.w3 = STHERM::RelayMode::ON;

    current_state = AppSpecCPP::SystemMode::EmergencyHeat;

    return true;
}

bool Relay::turnOffEmergencyHeating()
{
    mRelay.w1  = STHERM::RelayMode::OFF;
    mRelay.w2  = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    //! we are sure that everything is off as it may not proceed to next loop correctly
    current_state = AppSpecCPP::SystemMode::Off;

    return true;
}

void Relay::fanOn() {
    mRelay.g = STHERM::ON;
}

void Relay::fanOFF() {
    mRelay.g = STHERM::OFF;
}

void Relay::updateOB()
{
    if (ob_state == ob_on_state && ob_state != AppSpecCPP::SystemMode::Off) {
        mRelay.o_b = STHERM::RelayMode::ON;
    } else {
        mRelay.o_b = STHERM::RelayMode::OFF;
    }
}

AppSpecCPP::SystemMode Relay::getOb_on_state() const
{
    return ob_on_state;
}

void Relay::setOb_on_state(const AppSpecCPP::SystemMode &newOb_on_state)
{
    ob_on_state = newOb_on_state;
}

void Relay::setFanMode(bool on)
{
    mFanOn = on;
}

void Relay::updateFan()
{
    // The fan operates:
    //    if it's set to 'on' using either WPH or if either Y1 or W1 is activated.
    //   if at least one of the Humidity wirings is ON
    //   In emergency mode (w3 is ON), the fan should be active.
    if (mFanOn || mRelay.y1 == STHERM::ON || mRelay.w1 == STHERM::ON || mRelay.w3 == STHERM::ON ||
        mRelay.acc2 == STHERM::ON || mRelay.acc1n == STHERM::ON || mRelay.acc1p == STHERM::ON) {
        fanOn();

    } else {
        fanOFF();
    }
}

STHERM::RelayConfigs Relay::relays() {
    updateOB();
    updateFan();

    return mRelay;
}

STHERM::RelayConfigs Relay::relaysLast()
{
    return mRelayLast;
}

void Relay::setRelaysLast(STHERM::RelayConfigs last)
{
    mRelayLast = last;
}

int Relay::currentCoolingStage() {
    int currentCoolingStage = 0;
    if (currentState() == AppSpecCPP::SystemMode::Cooling) {
        currentCoolingStage = (mRelay.y1 == STHERM::RelayMode::ON) +
                              (mRelay.y2 == STHERM::RelayMode::ON);
    }

    return currentCoolingStage;
}

int Relay::currentHeatingStage() {
    int currentHeatingStage = 0;
    if (currentState() == AppSpecCPP::SystemMode::Heating ||
        currentState() == AppSpecCPP::SystemMode::EmergencyHeat||
        currentState() == AppSpecCPP::SystemMode::Emergency) {
        currentHeatingStage = (mRelay.y1 == STHERM::RelayMode::ON) +
                              (mRelay.y2 == STHERM::RelayMode::ON) +
                              (mRelay.w1 == STHERM::RelayMode::ON) +
                              (mRelay.w2 == STHERM::RelayMode::ON) +
                              (mRelay.w3 == STHERM::RelayMode::ON);

        // Just for safty
        if (currentHeatingStage > 3) {
            currentHeatingStage = 3;
        }
    }

    return currentHeatingStage;
}
