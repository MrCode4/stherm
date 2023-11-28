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
    before_state  = STHERM::SystemMode::Off;
    current_state = STHERM::SystemMode::Off;
    current_stage = 0;
}


void Relay::updateStates()
{
    if (mRelay.o_b == STHERM::RelayMode::NoWire) { // without OB
        if (mRelay.y1 == STHERM::RelayMode::ON) {
            before_state = STHERM::SystemMode::Cooling;
            current_stage = 1;
            if (mRelay.y2 == STHERM::RelayMode::ON) {
                current_stage = 2;
            }
            if (mRelay.y3 == STHERM::RelayMode::ON) {
                current_stage = 3;
            }
        } else if (mRelay.w1 == STHERM::RelayMode::ON) {
            before_state = STHERM::SystemMode::Heating;
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
        if (ob_state == STHERM::SystemMode::Cooling && mRelay.o_b == STHERM::RelayMode::ON) {
            before_state = STHERM::SystemMode::Cooling;
        } else if(ob_state == STHERM::SystemMode::Cooling && mRelay.o_b == STHERM::RelayMode::OFF) {
            before_state = STHERM::SystemMode::Heating;
        } else if (ob_state == STHERM::SystemMode::Heating && mRelay.o_b == STHERM::RelayMode::ON) {
            before_state = STHERM::SystemMode::Heating;
        }else if (ob_state == STHERM::SystemMode::Heating && mRelay.o_b == STHERM::RelayMode::OFF) {
            before_state = STHERM::SystemMode::Cooling;
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
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (ob_state == STHERM::SystemMode::Cooling) {
            mRelay.o_b = STHERM::RelayMode::ON;
        } else {
            mRelay.o_b = STHERM::RelayMode::OFF;
        }
        mRelay.y1 = STHERM::RelayMode::ON;
        mRelay.y2 = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;
    } else {

        mRelay.y1 = STHERM::RelayMode::ON;
        mRelay.y2 = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;
        mRelay.w1 = STHERM::RelayMode::OFF;
        mRelay.w2 = STHERM::RelayMode::OFF;
        mRelay.w3 = STHERM::RelayMode::OFF;
    }
    startTempTimer( STHERM::SystemMode::Cooling);
    current_state = STHERM::SystemMode::Cooling;

    return true;
}


void Relay::startTempTimer(STHERM::SystemMode current_state)
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
//    case STHERM::SystemMode::Cooling:
//        color = self::COOLING_RGBM;
//        break;
//    case 'emergency':
//        color = self::EMERGENCY_RGBM;
//        color_st = self::EMERGENCY_ST_RGBM;
//        break;
//    case STHERM::SystemMode::Heating:
//        color = self::HEATING_RGBM;
//        break;
    //    }
}

STHERM::SystemMode Relay::currentState() const
{
    return current_state;
}

void Relay::setHumidifierState(const bool on) {
    mRelay.hum_wiring = on ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
}

void Relay::setDehumidifierState(const bool on) {
    mRelay.hum_wiring = on ? STHERM::RelayMode::ON : STHERM::RelayMode::OFF;
}

STHERM::SystemMode Relay::getOb_state() const
{
    return ob_state;
}

void Relay::setOb_state(STHERM::SystemMode newOb_state)
{
    ob_state = newOb_state;
}

/**
     * Turn set mode Off
     * @return void
     */
void Relay::setAllOff()
{
    mRelay.g     = STHERM::RelayMode::OFF;
    mRelay.y1    = STHERM::RelayMode::OFF;
    mRelay.y2    = STHERM::RelayMode::OFF;
    mRelay.y3    = STHERM::RelayMode::OFF;
    mRelay.acc2  = STHERM::RelayMode::OFF;
    mRelay.w1    = STHERM::RelayMode::OFF;
    mRelay.w2    = STHERM::RelayMode::OFF;
    mRelay.w3    = STHERM::RelayMode::OFF;
    mRelay.o_b   = STHERM::RelayMode::OFF;
    mRelay.acc1p = STHERM::RelayMode::OFF;
    mRelay.acc1n = STHERM::RelayMode::OFF;
}

bool Relay::heatingStage0()
{
    if (ob_state == STHERM::SystemMode::Heating) {
        mRelay.o_b   = STHERM::RelayMode::ON;
    } else {
        mRelay.o_b   = STHERM::RelayMode::OFF;
    }
    mRelay.y1 = STHERM::RelayMode::OFF;
    mRelay.y2 = STHERM::RelayMode::OFF;
    mRelay.y3 = STHERM::RelayMode::OFF;

    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(STHERM::SystemMode::Heating);
    current_state = STHERM::SystemMode::Heating;

    return true;
}

bool Relay::coolingStage0()
{
    mRelay.o_b   = STHERM::RelayMode::OFF;
    if (ob_state == STHERM::SystemMode::Cooling) {
        mRelay.o_b   = STHERM::RelayMode::ON;
    } else {
        mRelay.o_b   = STHERM::RelayMode::OFF;
    }
    mRelay.y1  = STHERM::RelayMode::OFF;
    mRelay.y2  = STHERM::RelayMode::OFF;
    mRelay.y3 = STHERM::RelayMode::OFF;

    mRelay.w1 = STHERM::RelayMode::OFF;
    mRelay.w2 = STHERM::RelayMode::OFF;
    mRelay.w3 = STHERM::RelayMode::OFF;

    startTempTimer(STHERM::SystemMode::Cooling);
    current_state = STHERM::SystemMode::Cooling;

    return true;
}

bool Relay:: heatingStage1()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (ob_state == STHERM::SystemMode::Heating) {
            mRelay.o_b   = STHERM::RelayMode::ON;
        } else {
            mRelay.o_b   = STHERM::RelayMode::OFF;
        }
        mRelay.y1  = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;
    } else {
        mRelay.y1  = STHERM::RelayMode::OFF;
        mRelay.y2  = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;
        mRelay.w1  = STHERM::RelayMode::ON;
        mRelay.w2 = STHERM::RelayMode::OFF;
        mRelay.w3 = STHERM::RelayMode::OFF;
    }
    startTempTimer(STHERM::SystemMode::Heating);
    current_state = STHERM::SystemMode::Heating;

    return true;
}

bool Relay:: heatingStage2()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (ob_state == STHERM::SystemMode::Heating) {
            mRelay.o_b   = STHERM::RelayMode::ON;
        } else {
            mRelay.o_b   = STHERM::RelayMode::OFF;
        }

        mRelay.y1 = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::ON;
        mRelay.y3= STHERM::RelayMode::OFF;
    } else {
        mRelay.y1  = STHERM::RelayMode::OFF;
        mRelay.y2  = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;
        mRelay.w1  = STHERM::RelayMode::ON;
        mRelay.w2  = STHERM::RelayMode::ON;
        mRelay.w3 = STHERM::RelayMode::OFF;
    }
    startTempTimer(STHERM::SystemMode::Heating);
    current_state = STHERM::SystemMode::Heating;

    return true;
}

bool Relay::coolingStage2()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (ob_state == STHERM::SystemMode::Cooling) {
            mRelay.o_b   = STHERM::RelayMode::ON;
        } else {
            mRelay.o_b   = STHERM::RelayMode::OFF;
        }
        mRelay.y1  = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::ON;
        mRelay.y3 = STHERM::RelayMode::OFF;
    } else {
        mRelay.y1  = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::ON;
        mRelay.y3 = STHERM::RelayMode::OFF;
        mRelay.w1 = STHERM::RelayMode::OFF;
        mRelay.w2 = STHERM::RelayMode::OFF;
        mRelay.w3 = STHERM::RelayMode::OFF;
    }
    startTempTimer(STHERM::SystemMode::Cooling);
    current_state = STHERM::SystemMode::Cooling;
    return true;
}

bool Relay::heatingStage3()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (ob_state == STHERM::SystemMode::Heating) {
            mRelay.o_b   = STHERM::RelayMode::ON;
        } else {
            mRelay.o_b   = STHERM::RelayMode::OFF;
        }

        mRelay.y1  = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::ON;
        mRelay.y3  = STHERM::RelayMode::ON;
    } else {
        mRelay.w1  = STHERM::RelayMode::ON;
        mRelay.w2  = STHERM::RelayMode::ON;
        mRelay.w3  = STHERM::RelayMode::ON;

        mRelay.y1  = STHERM::RelayMode::OFF;
        mRelay.y2  = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;
    }
    startTempTimer(STHERM::SystemMode::Heating);
    current_state = STHERM::SystemMode::Heating;
    return true;
}

bool Relay::coolingStage3()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        if (ob_state == STHERM::SystemMode::Cooling) {
            mRelay.o_b   = STHERM::RelayMode::ON;
        } else {
            mRelay.o_b   = STHERM::RelayMode::OFF;
        }

        mRelay.y1  = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::ON;
        mRelay.y3  = STHERM::RelayMode::ON;
    } else {
        mRelay.y1  = STHERM::RelayMode::ON;
        mRelay.y2  = STHERM::RelayMode::ON;
        mRelay.y3  = STHERM::RelayMode::ON;
        mRelay.w1 = STHERM::RelayMode::OFF;
        mRelay.w2 = STHERM::RelayMode::OFF;
        mRelay.w3 = STHERM::RelayMode::OFF;
    }
    startTempTimer(STHERM::SystemMode::Cooling);
    current_state = STHERM::SystemMode::Cooling;

    return true;
}

bool Relay::emergencyHeating1()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        mRelay.y1  = STHERM::RelayMode::OFF;
        mRelay.y2  = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;

        mRelay.w1  = STHERM::RelayMode::ON;
        mRelay.w2 = STHERM::RelayMode::OFF;

        current_state = STHERM::SystemMode::Emergency;

        return true;
    }
    return false;
}

bool Relay::emergencyHeating2()
{
    if (mRelay.o_b != STHERM::RelayMode::NoWire) {
        mRelay.y1  = STHERM::RelayMode::OFF;
        mRelay.y2  = STHERM::RelayMode::OFF;
        mRelay.y3 = STHERM::RelayMode::OFF;

        mRelay.w1  = STHERM::RelayMode::ON;
        mRelay.w2  = STHERM::RelayMode::ON;

        current_state = STHERM::SystemMode::Emergency;

        return true;
    }

    return false;
}

bool Relay::turnOffEmergencyHeating()
{
    mRelay.w1  = STHERM::RelayMode::OFF;
    mRelay.w2  = STHERM::RelayMode::OFF;

    // update current_state

    return true;
}

void Relay::fanOn() {
    if (mRelay.g != STHERM::NoWire) {
        mRelay.g = STHERM::ON;
    }
}

void Relay::fanOFF() {
    if (mRelay.g != STHERM::NoWire) {
        mRelay.g = STHERM::OFF;
    }
}

bool Relay::fanWorkTime(int fanWPH, int interval)
{
    if (mRelay.y1 == STHERM::ON || mRelay.w1 == STHERM::ON) {
        fanOn();
        return true;
    }

    if (fanWPH > 0 && interval < 0 && interval >= -1 * fanWPH) {
        fanOn();
        return true;
    }

    fanOFF();
    return false;
}

STHERM::RelayConfigs Relay::relays() {
    return mRelay;
}
