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

bool Relay::fanWorkTime()
{
    qDebug() << Q_FUNC_INFO << __LINE__ ;
//    //$fan_settings = conn->getRow("SELECT fan_time AS user_set_time, start_fan_timing AS work_in_hour,(SELECT fan FROM current_state) AS user_set_interval, EXTRACT(MINUTE from (current_timestamp))-EXTRACT(MINUTE from (fan_time)) AS interval_minute, EXTRACT(MINUTE from (current_timestamp))-EXTRACT(MINUTE from (fan_time)) AS other FROM timing");
//    $interval = conn->getItem("SELECT fan FROM current_state");
//    if((int)$interval > 0) {
//        $fan_settings = conn->getRow("SELECT EXTRACT(MINUTE from ((current_timestamp - fan_time)- interval '{$interval} minute'))-1 AS work FROM timing");
//        getRelayState();
//        if (mRelay.y1 == 'on' || mRelay.w1 == 'on') {
//            fanOn();
//        } else {
//            if ((int)$interval > 0 && (int)$fan_settings['work'] < 0 && (int)$fan_settings['work'] >= (-1) * (int)$interval) {
//                fanOn();
//            } else { // fan auto mode
//                fanOff();
//            }
//        }
//    } else {
//        if (mRelay.y1 == 'on' || mRelay.w1 == 'on') {
//            fanOn();
//        }else{
//            fanOff();
//        }
//    }

//    if (mRelay['g'] != STHERM::RelayMode::NoWire) {
//        if (mRelay['g'] == 'on') {
//            conn->setQuery("UPDATE relays SET type = true WHERE alias = 'g';");
//        } else {
//            conn->setQuery("UPDATE relays SET type = false WHERE alias = 'g';");
//        }
//        getRelayState();
//    }
    return true;
}

STHERM::RelayConfigs Relay::relays() {
    return mRelay;
}
