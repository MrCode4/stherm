#include "Scheme.h"

#include <QColor>

#include "PhpApi.h"

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
const double STAGE2_OFF_RANGE        = 1.9; // F
const double STAGE3_OFF_RANGE        = 4.9;
const double ALERT_TIME              = 120;
const double CHANGE_STAGE_TIME       = 40;
const double CHANGE_STAGE_TIME_WO_OB = 10;
const double S2OFF_TIME              = 2;

// Status backlight color
const QVariantList coolingColor      = QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE, "true"};
const QVariantList heatingColor      = QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE,  "true"};
const QVariantList emergencyColor    = QVariantList{255, 0, 0, STHERM::LedEffect::LED_BLINK,  "true"};

Scheme::Scheme(QObject *parent) :
    QThread (parent)
{
    stopWork = false;

    mTiming = PhpAPI::instance()->timing();
    mRelay  = Relay::instance();

    connect(this, &Scheme::modeChanged, this, [this] {
        // todo: use a safe method
        if (this->isRunning()) {
            terminate();
            wait();
        }

        this->start();
    });
}

Scheme::~Scheme()
{
    stopWork = true;

    // Stop worker.
    terminate();
    wait();

}

void Scheme::run()
{
    while (!stopWork) {
        startWork();
    }
}


void Scheme::startWork()
{
    switch (mCurrentSysMode) {
   case STHERM::SystemMode::Cooling: {

       switch (mDeviceType) { // Device type
       case STHERM::CoolingType::Conventional:
       case STHERM::CoolingType::CoolingOnly: {
           if (mCurrentTemperature - mSetPointTemperature >= 1.9) {
               mRelay->setOb_state(STHERM::Heating);

               // sysDelay
               mRelay->coolingStage1();

               // 5 Sec
               emit changeBacklight(coolingColor);
               mTiming->s1uptime.restart();
               mTiming->s1uptime.restart();
               mTiming->s2hold = false;
               mTiming->alerts = false;

           } else {
               // turn off Y1, Y2 and G = 0
               mRelay->setAllOff();
               startWork();
           }
       }

       case STHERM::CoolingType::HeatPump: {
           if (mCurrentTemperature - mSetPointTemperature >= 1.9) {
               mRelay->setOb_state(STHERM::Cooling);

               // sysDelay
               mRelay->coolingStage1();

               // 5 Sec
               emit changeBacklight(coolingColor);
               mTiming->s1uptime.restart();
               mTiming->s1uptime.restart();
               mTiming->s2hold = false;
               mTiming->alerts = false;

           } else {
               // turn off Y1, Y2 and G = 0
               mRelay->setAllOff();
               startWork();
           }


       } break;

       default:
           break;

       }
   } break;

   case STHERM::SystemMode::Auto: {
       if (mCurrentTemperature > mSetPointTemperature ) {
           mRealSysMode = STHERM::SystemMode::Cooling;
       } else if (mCurrentTemperature < mSetPointTemperature) {
           mRealSysMode = STHERM::SystemMode::Heating;
       }

       // start work with new mode;
       startWork();
   } break;


   case STHERM::SystemMode::Heating: {
       switch (mDeviceType) {
       case STHERM::CoolingType::HeatPump: {
           if(mCurrentTemperature < mSetPointTemperature) {
               if (mCurrentTemperature < ET) {
                   heatingEmergencyHeatPumpRole1();
               } else {
                   heatingHeatPumpRole1();
               }
           } else {
               // Turn off system (Y1, Y2, W1, W2,W3, G = 0)
               mRelay->setAllOff();
               startWork();
               return;
           }
       }
       case STHERM::CoolingType::Conventional:
       case STHERM::CoolingType::HeatingOnly: {
           if (mSetPointTemperature - mCurrentTemperature >= 1.9) {
               // Sys delay
               mRelay->heatingStage1();

               mCurrentSysMode = mRelay->currentState();
               // 5 secs
               emit changeBacklight(heatingColor);

               mTiming->s1uptime.restart();
               mTiming->uptime.restart();
               mTiming->s2hold = false;
               mTiming->s3hold = false;
               mTiming->alerts = false;

               heatingConventionalRole1();
               return;

           } else {
               mRelay-> setAllOff();
               startWork();
           }
       }

       default:
           break;
       }

   } break;

   case STHERM::SystemMode::Vacation: {
       updateVacationState();
   } break;

   default:
       break;
   }
}

void Scheme::heatingConventionalRole1(bool needToWait)
{
   if (needToWait) {
       auto loopResult = waitLoop();

       if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
           startWork();
           return;
       }
   }

   if (mCurrentTemperature - mSetPointTemperature >= 1) {
       // turn of system (w1, w2, w3 = 0)
       mRelay->setAllOff();
       startWork();

       return;

   } else {
       if (mRelay->relays().w2 == STHERM::RelayMode::ON) {

           if (mSetPointTemperature - mCurrentTemperature >= 2.9 || (mTiming->s2uptime.isValid() && mTiming->s1uptime.elapsed() / 60000 >= 10)) {
               mRelay->heatingStage2();
               mCurrentSysMode = mRelay->currentState();
               // 5 secs
               emit changeBacklight(heatingColor);

               mTiming->s1uptime.invalidate();
               heatingConventionalRole2();

           } else {
               heatingConventionalRole1();
           }

           return;
       }

       // Generate Alert
       if (!mTiming->alerts && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           mTiming->alerts = true;
       }

       heatingConventionalRole1();
   }
}

void Scheme::heatingConventionalRole2()
{
   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   if (mTiming->s2hold) {
       if (mCurrentTemperature - mSetPointTemperature >= 1) {
           mRelay->setAllOff();
           startWork();
           return;
       } else if (mRelay->relays().w3 == STHERM::RelayMode::ON) {
           if ((mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() / 60000 >= 10)) {
               mRelay->heatingStage3();
               mCurrentSysMode = mRelay->currentState();
               // 5 secs
               emit changeBacklight(heatingColor);

               mTiming->s2hold = false;
               heatingConventionalRole3();
               return;
           }
       }

   } else {
       if (mSetPointTemperature - mCurrentTemperature < 8 || mRelay->relays().w3 != STHERM::RelayMode::ON) {
           if (mSetPointTemperature - mCurrentTemperature < 1.9) {
               mRelay->setAllOff();
               mRelay->heatingStage1();
               mCurrentSysMode = mRelay->currentState();
               // 5 secs
               emit changeBacklight(heatingColor);

               mTiming->s2hold = true;
               mTiming->s1uptime.invalidate();

               heatingConventionalRole1(false);
               return;

           }
       } else {
           if (mRelay->relays().w3 == STHERM::RelayMode::ON) {
               if (mSetPointTemperature - mCurrentTemperature >= 5.9 || (mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() / 60000 >= 10)) {
                   mRelay->heatingStage3();
                   mCurrentSysMode = mRelay->currentState();
                   // 5 secs
                   emit changeBacklight(heatingColor);

                   mTiming->s2hold = false;
                   heatingConventionalRole3();

                   return;
               }
           }
       }
   }

   if (!mTiming->alerts && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() / 60000 >= 120)) {
       emit alert();
       mTiming->alerts = true;
   }

   heatingConventionalRole2();
}

void Scheme::heatingConventionalRole3()
{
   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   if (mTiming->s3hold) {
       if (mCurrentTemperature - mSetPointTemperature>= 1) {
           mRelay->setAllOff();
           startWork();
           return;
       }
   } else if (mSetPointTemperature - mCurrentTemperature < 4.9) {
       // Turn off stage 3 keep stage 3 run (w3 = 0, W1, W2, G = 1)
       mRelay->setAllOff();
       mRelay->heatingStage2();

       mCurrentSysMode = mRelay->currentState();
       // 5 secs
       emit changeBacklight(heatingColor);

       mTiming->s1uptime.invalidate();
       mTiming->s2uptime.invalidate();
       mTiming->s3hold = false;

       heatingConventionalRole2();
       return;
   }

   // Generate Alert
   if (!mTiming->alerts && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() / 60000 >= 120)) {
       emit alert();
       mTiming->alerts = true;
   }

   heatingConventionalRole3();
}

void Scheme::heatingEmergencyHeatPumpRole1()
{
   // Y1, Y2, G = 0
   mRelay->setAllOff();
   mRelay->emergencyHeating1();
   mCurrentSysMode = mRelay->currentState();

   // untile the end of emergency mode.
   emit changeBacklight(emergencyColor, -1);

   heatingEmergencyHeatPumpRole2();
}

void Scheme::heatingEmergencyHeatPumpRole2()
{
   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;


   if (ct < ET - 3.5) {
       mRelay->emergencyHeating2();
       mCurrentSysMode = mRelay->currentState();
       heatingEmergencyHeatPumpRole3();
   } else if (ct < HPT) {
       heatingEmergencyHeatPumpRole2();
       return;
   } else {
       mRelay->setAllOff();
       heatingHeatPumpRole1();
       return;

   }
}

void Scheme::heatingEmergencyHeatPumpRole3()
{
   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   if (mCurrentTemperature < HPT) {
       heatingEmergencyHeatPumpRole3();
   } else {
       mRelay->turnOffEmergencyHeating();
       heatingHeatPumpRole1();
   }
}

void Scheme::heatingHeatPumpRole1()
{
   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   if (mSetPointTemperature - mCurrentTemperature >= 3) {
       mRelay->setOb_state(STHERM::Heating);

       //SysDelay
       mRelay->heatingStage1();
       mCurrentSysMode = mRelay->currentState();

       // 5 secs
       emit changeBacklight(heatingColor);

       mTiming->s1uptime.restart();
       mTiming->uptime.restart();
       mTiming->s2hold = false;
       mTiming->alerts = false;

       heatingHeatPumpRole2();
   } else {
       mRelay->setAllOff();
       startWork();
       return;
   }
}

void Scheme::heatingHeatPumpRole2(bool needToWait)
{
   if (needToWait) {
       auto loopResult = waitLoop();

       if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
           startWork();
           return;
       }
   }

   if (mCurrentTemperature - mSetPointTemperature >= 1.9) {
       mRelay->setAllOff();
       startWork();
       return;
   } else {
       if (mRelay->relays().y2 == STHERM::RelayMode::ON) {
           if (mSetPointTemperature - mCurrentTemperature >= 2.9 || (mTiming->uptime.isValid() && mTiming->s1uptime.elapsed() / 60000 >= 40)) {
               if ((mTiming->s2Offtime.isValid() && mTiming->s2Offtime.elapsed() / 60000 >= 2)) {
                   mRelay->heatingStage2();
                   mCurrentSysMode = mRelay->currentState();

                   // 5 secs
                   emit changeBacklight(heatingColor);

                   return;
               } else {
                   heatingHeatPumpRole2();
                   return;
               }
           } else {
               heatingHeatPumpRole2();
               return;
           }

       } else if (!mTiming->alerts && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           mTiming->alerts = true;
           heatingHeatPumpRole2();
           return;
       }
   }
}

void Scheme::heatingHeatPumpRole3()
{

   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   if (mTiming->s2hold) {
       if (mCurrentTemperature - mSetPointTemperature >= 1) {
           mRelay->setAllOff();
           startWork();
           return;
       } else if (!mTiming->alerts && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           mTiming->alerts = true;
           heatingHeatPumpRole3();
           return;

       } else {
           heatingHeatPumpRole3();
       }

   } else {
       if (mSetPointTemperature - mCurrentTemperature <= 1.9) {
           // Y2 = 0, Y1 and G = 1
           mRelay->setAllOff();
           mRelay->heatingStage1();
           mCurrentSysMode = mRelay->currentState();

           // 5 secs
           emit changeBacklight(heatingColor);

           mTiming->s1uptime.restart();
           mTiming->s2Offtime.restart();
           mTiming->s2hold = true;

           heatingHeatPumpRole2(false);

           return;

       } else if (!mTiming->alerts && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           mTiming->alerts = true;
           heatingHeatPumpRole3();
           return;
       } else {
           heatingHeatPumpRole3();
       }
   }
}

void Scheme::coolingHeatPumpRole1(bool needToWait)
{
   if (needToWait) {
       auto loopResult = waitLoop();

       if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
           startWork();
           return;
       }
   }

   if (mSetPointTemperature - mCurrentTemperature >= 1) {
       // turn off Y1, Y2 and G = 0
       mRelay->setAllOff();
       startWork();

   } else {
       if (mRelay->relays().y2 == STHERM::RelayMode::ON) {
           if (mCurrentTemperature - mSetPointTemperature >= 2.9 || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() / 60000 >= 40)) {
               if (mTiming->s2Offtime.isValid() && mTiming->s2Offtime.elapsed() / 60000 >= 2) {
                   // turn on stage 2
                   mRelay->coolingStage2();
                   // 5 Sec
                   emit changeBacklight(coolingColor);

                   coolingHeatPumpRole2();

               } else {
                   coolingHeatPumpRole1();
                   return;
               }

           } else {
               coolingHeatPumpRole1();
               return;
           }
       } else if (!mTiming->alerts && mTiming->uptime.elapsed() / 60000 >= 120) {
           emit alert();
           mTiming->alerts = true;

           coolingHeatPumpRole1();
           return;

       } else {
           coolingHeatPumpRole1();
           return;

       }
   }
}

void Scheme::coolingHeatPumpRole2()
{
   auto loopResult = waitLoop();

   if (loopResult == ChangeType::Mode || loopResult == ChangeType::SetTemperature) {
       startWork();
       return;
   }

   if (mTiming->s2hold) {
       if (mSetPointTemperature - mCurrentTemperature >= 1) {
           mRelay->setAllOff();
           startWork();
       } else {
           if (!mTiming->alerts && mTiming->uptime.elapsed() / 60000 >= 120) {
               emit alert();
               mTiming->alerts = true;
           }
           coolingHeatPumpRole2();
       }
   } else {
       if (mCurrentTemperature - mSetPointTemperature <= 1.9) {
//           mRelay->relays().y2 = STHERM::OFF;
           mRelay->coolingStage1();

           // 5 secs
           emit changeBacklight(coolingColor);
           mTiming->s1uptime.invalidate();// = 0;
           mTiming->s2hold = true;
           // Start to count s2 off time
           mTiming->s2Offtime.restart();

           // may be incorrect, must be call coolingHeatPumpRole1 with true
           coolingHeatPumpRole1(false);
       }
   }
}

int Scheme::waitLoop() {
   QEventLoop loop;
   connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           loop.exit(ChangeType::CurrentTemperature);

       }, Qt::SingleShotConnection);

   return loop.exec();
}

double Scheme::currentHumidity() const
{
   return mCurrentHumidity;
}

void Scheme::setCurrentHumidity(double newCurrentHumidity)
{
   if (mCurrentHumidity != newCurrentHumidity)
       return;

   mCurrentHumidity = newCurrentHumidity;

   updateHumifiresState();
}

double Scheme::setPointHimidity() const
{
   return mSetPointHimidity;
}

void Scheme::setSetPointHimidity(double newSetPointHimidity)
{
   mSetPointHimidity = newSetPointHimidity;
}

void Scheme::setMainData(QVariantMap mainData)
{
    if (_mainData == mainData)
        return;

    _mainData = mainData;

    bool isOk;
    double currentTemp = mainData.value("temperature").toDouble(&isOk);

    if (isOk && currentTemp != mCurrentTemperature) {
        mCurrentTemperature = currentTemp;

        emit currentTemperatureChanged();
    }

    double currentHumidity =mainData.value("humidity").toDouble(&isOk);

    if (isOk) {
        setCurrentHumidity(currentHumidity);
    }
}

void Scheme::updateRealState(const struct STHERM::Vacation &vacation, const double &setTemperature, const double &currentTemperature, const double &currentHumidity)
{
    if (mRealSysMode == STHERM::SystemMode::Cooling) {
       mRealSysMode = updateNormalState(setTemperature, currentTemperature, currentHumidity);

    } else if (mRealSysMode == STHERM::SystemMode::Vacation) {
//       mRealSysMode = updateVacationState(vacation, setTemperature, currentTemperature, currentHumidity);

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

    if (mRealSysMode != mCurrentSysMode) { // mode changes
//       current_stage = 0;
    }


}

void Scheme::updateVacationState()
{
    if (mCurrentSysMode == STHERM::SystemMode::Cooling) {
        if (mCurrentTemperature > mSetPointTemperature - STAGE1_OFF_RANGE) { // before stage 1 off
           mRealSysMode = STHERM::SystemMode::Cooling;
        } else if (mCurrentTemperature > mSetPointTemperature - STAGE1_ON_RANGE) { // before stage 1 on
           mRealSysMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
           mRealSysMode = STHERM::SystemMode::Heating;
        }
    } else if (mCurrentSysMode == STHERM::SystemMode::Heating) {
        if (mCurrentTemperature < mSetPointTemperature + STAGE1_OFF_RANGE) { // before stage 1 off
           mRealSysMode = STHERM::SystemMode::Heating;
        } else if (mCurrentTemperature < mSetPointTemperature + STAGE1_ON_RANGE) { // before stage 1 on
           mRealSysMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            mRealSysMode = STHERM::SystemMode::Cooling;
        }
    } else { // OFF
        if (mCurrentTemperature < mSetPointTemperature - STAGE1_ON_RANGE) {
            mRealSysMode = STHERM::SystemMode::Heating;
        } else if (mCurrentTemperature > mSetPointTemperature + STAGE1_ON_RANGE) {
            mRealSysMode = STHERM::SystemMode::Cooling;
        }
    }

    if (mVacation.minimumTemperature > mCurrentTemperature) {
        mRealSysMode = STHERM::SystemMode::Heating;
        //        range = temperature - $current_state['min_temp'];
    } else if (mVacation.maximumTemperature < mCurrentTemperature) {
        mRealSysMode = STHERM::SystemMode::Cooling;
        //        range = temperature - current_state['max_temp'];
    }

    updateHumifiresState();

    // Update current system mode
    mCurrentSysMode = mRealSysMode;


    // Start work
    startWork();
}

void Scheme::updateHumifiresState()
{
    if (mHumidifierId == 3)
        return;

    if (mCurrentSysMode == STHERM::Vacation) {
        if (mHumidifierId == 1) {
            setHumidifierState(mCurrentHumidity < mVacation.minimumHumidity);

        } else if (mHumidifierId == 2) {
            setDehumidifierState(mCurrentHumidity > mVacation.maximumHumidity);
        }

    } else {
        double max_hum = HUM_MAX;
        double min_hum = HUM_MIN;

        if (mSetPointHimidity + HUM_STEP < max_hum)
            max_hum = mSetPointHimidity + HUM_STEP;

        if (mSetPointHimidity - HUM_STEP > min_hum)
            min_hum = mSetPointHimidity - HUM_STEP;

        if (mHumidifierId == 1) {
            if (mCurrentHumidity < mSetPointHimidity) // on
               setHumidifierState(true);

            else if (mCurrentHumidity >= max_hum)    // off
               setHumidifierState(false);

        } else if (mHumidifierId == 2) { // dehumidifier
            if (mCurrentHumidity > mSetPointHimidity)
               setDehumidifierState(true);
            else if (mCurrentHumidity <= min_hum)
               setDehumidifierState(false);
        }
    }
}

STHERM::SystemMode Scheme::updateNormalState(const double &setTemperature, const double &currentTemperature, const double &currentHumidity)
{
    STHERM::SystemMode realSetMode;

    if (mCurrentSysMode == STHERM::SystemMode::Cooling) {
        if (currentTemperature > setTemperature - STAGE1_OFF_RANGE) { // before stage 1 off
            realSetMode = STHERM::SystemMode::Cooling;
        } else if (currentTemperature > setTemperature - STAGE1_ON_RANGE) { // before stage 1 on
            realSetMode = STHERM::SystemMode::Off;
        } else {  // stage 1 on
            realSetMode = STHERM::SystemMode::Heating;
        }
    } else if (mCurrentSysMode == STHERM::SystemMode::Heating) {
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

void Scheme::setHumidifierId(const int &humidifierId)
{
    if (mHumidifierId != humidifierId)
        return;

    mHumidifierId = humidifierId;
    updateHumifiresState();

}

STHERM::SystemMode Scheme::getCurrentSysMode() const
{
    return mCurrentSysMode;
}

void Scheme::setCurrentSysMode(STHERM::SystemMode newSysMode)
{
    if (mCurrentSysMode == newSysMode)
        return;

    mCurrentSysMode = newSysMode;
}

void Scheme::setHumidifierState(bool on)
{

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
