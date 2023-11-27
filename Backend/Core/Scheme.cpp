#include "Scheme.h"

#include <QColor>

#include "PhpApi.h"
#include "Relay.h"
#include "include/timing.h"

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

Scheme::Scheme(QObject *parent) :
    QThread (parent)
{
    stopWork = false;
}

void Scheme::startWork2()
{

}

void Scheme::run()
{
    while (!stopWork) {

    }

}


void Scheme::startWork()
{
    double sp = 0.0; // Set point temperature
    double ct = mCurrentTemperature;

    auto timing = PhpAPI::instance()->timing();
    auto relay  = Relay::instance();
    auto relaysConfig = relay->relays();

   switch (mRealSysMode) {
   case STHERM::SystemMode::Cooling: {

       switch (mDeviceType) { // Device type
       case STHERM::CoolingType::Conventional:
       case STHERM::CoolingType::CoolingOnly: {
           if (ct - sp >= 1.9) {
               relay->setOb_state(STHERM::Heating);

               // sysDelay
               relay->coolingStage1();

               // 5 Sec
               emit changeBacklight(QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE});
               timing->s1uptime.restart();
               timing->s1uptime.restart();
               timing->s2hold = false;
               timing->alerts = false;

           } else {
               // turn off Y1, Y2 and G = 0
               relay->setAllOff();
               startWork();
           }
       }

       case STHERM::CoolingType::HeatPump: {
           if (ct - sp >= 1.9) {
               relay->setOb_state(STHERM::Cooling);

               // sysDelay
               relay->coolingStage1();

               // 5 Sec
               emit changeBacklight(QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE});
               timing->s1uptime.restart();
               timing->s1uptime.restart();
               timing->s2hold = false;
               timing->alerts = false;

           } else {
               // turn off Y1, Y2 and G = 0
               relay->setAllOff();
               startWork();
           }


       } break;

       default:
           break;

       }
   } break;

   case STHERM::SystemMode::Auto: {
       if (ct > sp ) {
           mRealSysMode = STHERM::SystemMode::Cooling;
       } else if (ct < sp) {
           mRealSysMode = STHERM::SystemMode::Heating;
       }

       // start work with new mode;
       startWork();
   } break;


   case STHERM::SystemMode::Heating: {
       switch (mDeviceType) {
       case STHERM::CoolingType::HeatPump: {
           if(ct < sp) {
               if (ct < ET) {
                   heatingEmergencyHeatPumpRole1();
               } else {
                   heatingHeatPumpRole1();
               }
           } else {
               // Turn off system (Y1, Y2, W1, W2,W3, G = 0)
               relay->setAllOff();
               startWork();
               return;
           }
       }
       case STHERM::CoolingType::Conventional:
       case STHERM::CoolingType::HeatingOnly: {
           if (sp - ct >= 1.9) {
               // Sys delay
               relay->heatingStage1();

               mCurentSysMode = relay->currentState();
               // 5 secs
               emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

               timing->s1uptime.restart();
               timing->uptime.restart();
               timing->s2hold = false;
               timing->s3hold = false;
               timing->alerts = false;

               heatingConventionalRole1();
               return;

           } else {
               relay-> setAllOff();
               startWork();
           }
       }

       default:
           break;
       }

   } break;

   default:
       break;
   }
}

void Scheme::heatingConventionalRole1(bool needToWait)
{
   if (needToWait) {
       //! wait to change temperatre
       QEventLoop loop;
       loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
               qDebug() << Q_FUNC_INFO << __LINE__ ;
               loop.quit();
           }, Qt::SingleShotConnection);
       loop.exec();
   }

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (ct - sp >= 1) {
       // turn of system (w1, w2, w3 = 0)
       relay->setAllOff();
       startWork();

       return;

   } else {
       if (relaysConfig.w2 == STHERM::RelayMode::ON) {

           if (sp - ct >= 2.9 || (timing->s2uptime.isValid() && timing->s1uptime.elapsed() / 60000 >= 10)) {
               relay->heatingStage2();
               mCurentSysMode = relay->currentState();
               // 5 secs
               emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

               timing->s1uptime.invalidate();
               heatingConventionalRole2();

           } else {
               heatingConventionalRole1();
           }

           return;
       }

       // Generate Alert
       if (!timing->alerts && (timing->uptime.isValid() && timing->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           timing->alerts = true;
       }

       heatingConventionalRole1();
   }
}

void Scheme::heatingConventionalRole2()
{
   //! wait to change temperatre
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (timing->s2hold) {
       if (ct - sp >= 1) {
           relay->setAllOff();
           startWork();
           return;
       } else if (relaysConfig.w3 == STHERM::RelayMode::ON) {
           if ((timing->s2uptime.isValid() && timing->s2uptime.elapsed() / 60000 >= 10)) {
               relay->heatingStage3();
               mCurentSysMode = relay->currentState();
               // 5 secs
               emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

               timing->s2hold = false;
               heatingConventionalRole3();
               return;
           }
       }

   } else {
       if (sp - ct < 8 || relaysConfig.w3 != STHERM::RelayMode::ON) {
           if (sp - ct < 1.9) {
               relay-> setAllOff();
               relay->heatingStage1();
               mCurentSysMode = relay->currentState();
               // 5 secs
               emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

               timing->s2hold = true;
               timing->s1uptime.invalidate();

               heatingConventionalRole1(false);
               return;

           }
       } else {
           if (relaysConfig.w3 == STHERM::RelayMode::ON) {
               if (sp - ct >= 5.9 || (timing->s2uptime.isValid() && timing->s2uptime.elapsed() / 60000 >= 10)) {
                   relay->heatingStage3();
                   mCurentSysMode = relay->currentState();
                   // 5 secs
                   emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

                   timing->s2hold = false;
                   heatingConventionalRole3();

                   return;
               }
           }
       }
   }

   if (!timing->alerts && (timing->uptime.isValid() && timing->uptime.elapsed() / 60000 >= 120)) {
       emit alert();
       timing->alerts = true;
   }

   heatingConventionalRole2();
}

void Scheme::heatingConventionalRole3()
{
   //! wait to change temperatre
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (timing->s3hold) {
       if (ct - sp>= 1) {
           relay->setAllOff();
           startWork();
           return;
       }
   } else if (sp - ct < 4.9) {
       // Turn off stage 3 keep stage 3 run (w3 = 0, W1, W2, G = 1)
       relay->setAllOff();
       relay->heatingStage2();

       mCurentSysMode = relay->currentState();
       // 5 secs
       emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

       timing->s1uptime.invalidate();
       timing->s2uptime.invalidate();
       timing->s3hold = false;

       heatingConventionalRole2();
       return;
   }

   // Generate Alert
   if (!timing->alerts && (timing->uptime.isValid() && timing->uptime.elapsed() / 60000 >= 120)) {
       emit alert();
       timing->alerts = true;
   }

   heatingConventionalRole3();
}

void Scheme::heatingEmergencyHeatPumpRole1()
{
   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   // Y1, Y2, G = 0
   relay->setAllOff();
   relay->emergencyHeating1();
   mCurentSysMode = relay->currentState();

   // untile the end of emergency mode.
   emit changeBacklight(QVariantList{255, 0, 0, STHERM::LedEffect::LED_BLINK});

   heatingEmergencyHeatPumpRole2();
}

void Scheme::heatingEmergencyHeatPumpRole2()
{
   //! wait to change temperatre
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (ct < ET - 3.5) {
       relay->emergencyHeating2();
       mCurentSysMode = relay->currentState();
       heatingEmergencyHeatPumpRole3();
   } else if (ct < HPT) {
       heatingEmergencyHeatPumpRole2();
       return;
   } else {
       relay->setAllOff();
       heatingHeatPumpRole1();
       return;

   }
}

void Scheme::heatingEmergencyHeatPumpRole3()
{
   //! wait to change temperatre
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (ct < HPT) {
       heatingEmergencyHeatPumpRole3();
   } else {
       relay->setAllOff();
       heatingHeatPumpRole1();
   }
}

void Scheme::heatingHeatPumpRole1()
{
   //! wait to change temperatre
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (sp - ct >= 3) {
       relay->setOb_state(STHERM::Heating);

       //SysDelay
       relay->heatingStage1();
       mCurentSysMode = relay->currentState();

       // 5 secs
       emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

       timing->s1uptime.restart();
       timing->uptime.restart();
       timing->s2hold = false;
       timing->alerts = false;

       heatingHeatPumpRole2();
   } else {
       relay->setAllOff();
       startWork();
       return;
   }
}

void Scheme::heatingHeatPumpRole2(bool needToWait)
{
   if (needToWait) {
       //! wait to change temperatre
       QEventLoop loop;
       loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
               qDebug() << Q_FUNC_INFO << __LINE__ ;
               loop.quit();
           }, Qt::SingleShotConnection);
       loop.exec();
   }

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (ct - sp >= 1.9) {
       relay->setAllOff();
       startWork();
       return;
   } else {
       if (relaysConfig.y2 == STHERM::RelayMode::ON) {
           if (sp - ct >= 2.9 || (timing->uptime.isValid() && timing->s1uptime.elapsed() / 60000 >= 40)) {
               if ((timing->s2Offtime.isValid() && timing->s2Offtime.elapsed() / 60000 >= 2)) {
                   relay->heatingStage2();
                   mCurentSysMode = relay->currentState();

                   // 5 secs
                   emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

                   return;
               } else {
                   heatingHeatPumpRole2();
                   return;
               }
           } else {
               heatingHeatPumpRole2();
               return;
           }

       } else if (!timing->alerts && (timing->uptime.isValid() && timing->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           timing->alerts = true;
           heatingHeatPumpRole2();
           return;
       }
   }
}

void Scheme::heatingHeatPumpRole3()
{
   //! wait to change temperatre
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (timing->s2hold) {
       if (ct - sp >= 1) {
           relay->setAllOff();
           startWork();
           return;
       } else if (!timing->alerts && (timing->uptime.isValid() && timing->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           timing->alerts = true;
           heatingHeatPumpRole3();
           return;

       } else {
           heatingHeatPumpRole3();
       }

   } else {
       if (sp - ct <= 1.9) {
           // Y2 = 0, Y1 and G = 1
           relay->setAllOff();
           relay->heatingStage1();
           mCurentSysMode = relay->currentState();

           // 5 secs
           emit changeBacklight(QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE});

           timing->s1uptime.restart();
           timing->s2Offtime.restart();
           timing->s2hold = true;

           heatingHeatPumpRole2(false);

           return;

       } else if (!timing->alerts && (timing->uptime.isValid() && timing->uptime.elapsed() / 60000 >= 120)) {
           emit alert();
           timing->alerts = true;
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
       QEventLoop loop;
       loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
               qDebug() << Q_FUNC_INFO << __LINE__ ;
               loop.quit();
           }, Qt::SingleShotConnection);
       loop.exec();
   }

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (sp - ct >= 1) {
       // turn off Y1, Y2 and G = 0
       relay->setAllOff();
       startWork();

   } else {
       if (relaysConfig.y2 == STHERM::RelayMode::ON) {
           if (ct - sp >= 2.9 || (timing->s1uptime.isValid() && timing->s1uptime.elapsed() / 60000 >= 40)) {
               if (timing->s2Offtime.isValid() && timing->s2Offtime.elapsed() / 60000 >= 2) {
                   // turn on stage 2
                   relay->coolingStage2();
                   // 5 Sec
                   emit changeBacklight(QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE});

                   coolingHeatPumpRole2();

               } else {
                   coolingHeatPumpRole1();
                   return;
               }

           } else {
               coolingHeatPumpRole1();
               return;
           }
       } else if (!timing->alerts && timing->uptime.elapsed() / 60000 >= 120) {
           emit alert();
           timing->alerts = true;

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
   // To monitor temperature
   QEventLoop loop;
   loop.connect(this, &Scheme::currentTemperatureChanged, this, [&loop]() {
           qDebug() << Q_FUNC_INFO << __LINE__ ;
           loop.quit();
       }, Qt::SingleShotConnection);
   loop.exec();

   double sp = 0.0; // Set point temperature
   double ct = mCurrentTemperature;

   auto timing = PhpAPI::instance()->timing();
   auto relay  = Relay::instance();
   auto relaysConfig = relay->relays();

   if (timing->s2hold) {
       if (sp - ct >= 1) {
           relay->setAllOff();
           startWork();
       } else {
           if (!timing->alerts && timing->uptime.elapsed() / 60000 >= 120) {
               emit alert();
               timing->alerts = true;
           }
           coolingHeatPumpRole2();
       }
   } else {
       if (ct - sp <= 1.9) {
//           relaysConfig.y2 = STHERM::OFF;
           relay->coolingStage1();

           // 5 secs
           emit changeBacklight(QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE});
           timing->s1uptime.invalidate();// = 0;
           timing->s2hold = true;
           // Start to count s2 off time
           timing->s2Offtime.restart();

           // may be incorrect, must be call coolingHeatPumpRole1 with true
           coolingHeatPumpRole1(false);
       }
   }
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
