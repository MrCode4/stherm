#include "Scheme.h"

#include <QColor>
#include <QEventLoop>
#include <QTimer>

#include "LogHelper.h"

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
const QVariantList emergencyColorS   = QVariantList{255, 0, 0, STHERM::LedEffect::LED_STABLE, "true"};

Scheme::Scheme(DeviceAPI* deviceAPI, QObject *parent) :
    mDeviceAPI(deviceAPI),
    QThread (parent)
{
    stopWork = true;

    mTiming = mDeviceAPI->timing();
    mRelay  = Relay::instance();

    mCurrentSysMode = AppSpecCPP::SystemMode::Auto;
}

Scheme::~Scheme()
{
    stopWork = true;

    // Stop worker.
    terminate();
    wait();

}

void Scheme::restartWork()
{
    if (isRunning()) {
        TRACE << "restarting HVAC" << stopWork;
        if (stopWork) // restart is already in progress
            return;
        // Any finished signal should not start the worker.
        connect(
            this,
            &Scheme::finished,
            this,
            [=]() {
                TRACE << "restarted HVAC";
                stopWork = false;
                this->start();
            },
            Qt::SingleShotConnection);

        stopWork = true;
        emit stopWorkRequested();
        this->wait(QDeadlineTimer(1000, Qt::PreciseTimer));

    } else {
        TRACE << "started HVAC";
        stopWork = false;
        this->start();
    }
}

void Scheme::setSetPointTemperature(double newSetPointTemperature)
{
     newSetPointTemperature = 32.0 + newSetPointTemperature * 9 / 5;
    if (qAbs(mSetPointTemperature - newSetPointTemperature) < 0.001)
        return;

    mSetPointTemperature = newSetPointTemperature;

    TRACE << "mSetPointTemperature changed";
}

void Scheme::setOrgSetTemperature(double newSetPointTemperature)
{
    mOriginalSetPointTemperature = newSetPointTemperature;

    setSetPointTemperature(mOriginalSetPointTemperature);

}

void Scheme::setRequestedHumidity(double newHumidity)
{
    if (qAbs(mSetPointHimidity - newHumidity) < 0.001)
        return;

    mSetPointHimidity = newHumidity;

    // Restart is not necessary
}

void Scheme::run()
{
    TRACE << "-- startWork is running." << QThread::currentThreadId();

    QElapsedTimer timer;
    timer.start();

    if (!mSystemSetup) {
        TRACE << "-- mSystemSetup is not ready.";
        return;
    }

    //! get wires config type modes ...
    updateParameters();

    //! reset delays
    resetDelays();

    while (!stopWork) {
        // Set temperature to original value
        setSetPointTemperature(mOriginalSetPointTemperature);

        // where should schedule be handled
        if (mSystemSetup->isVacation) {
            VacationLoop();

        } else {

            switch (mSystemSetup->systemMode) {
            case AppSpecCPP::SystemMode::Auto:
                AutoModeLoop();
                break;
            case AppSpecCPP::SystemMode::Cooling:
                CoolingLoop();
                break;
            case AppSpecCPP::SystemMode::Heating:
                HeatingLoop();
                break;
            case AppSpecCPP::SystemMode::Vacation:
                break;
            case AppSpecCPP::SystemMode::Off:
                OffLoop();
                break;
            case AppSpecCPP::SystemMode::Emergency:
                EmergencyLoop();
                break;
            default:
                qWarning() << "Unsupported Mode in controller loop" << mSystemSetup->systemMode;
                break;
            }
        }

        mRelay->setAllOff();

        //        int fanWork = QDateTime::currentSecsSinceEpoch() - mTiming->fan_time.toSecsSinceEpoch() - mFanWPH - 1;
        //        mRelay->fanWorkTime(mFanWPH, fanWork);

        sendRelays();

        if (stopWork)
            break;

        // all should be off! we can assert here
        waitLoop();
    }

    TRACE << "-- startWork Stopped. working time(ms): " << timer.elapsed();
}

void Scheme::updateParameters()
{
    //TODO
    // realSysMode from schedule or vacation
    // wire!?! setAllOff for now
}

void Scheme::resetDelays()
{
    //TODO
}

void Scheme::AutoModeLoop()
{
    if (mCurrentTemperature > mSetPointTemperature) {
        CoolingLoop();
    } else if (mCurrentTemperature < mSetPointTemperature) {
        HeatingLoop();
    }
}

void Scheme::CoolingLoop()
{
    TRACE_CHECK(true) << "Cooling started " << mSystemSetup->systemType;

    bool heatPump = false;
    // s1 time threshold
    // Y1, Y2, O/B G config

    switch (mSystemSetup->systemType) { // Device type
    case AppSpecCPP::SystemType::HeatPump:
        heatPump = true;
    case AppSpecCPP::SystemType::Conventional:
    case AppSpecCPP::SystemType::CoolingOnly: {
        TRACE << heatPump << mCurrentTemperature << mSetPointTemperature;
        if (mCurrentTemperature - mSetPointTemperature >= 1.9) {
            internalCoolingLoopStage1(heatPump);
        }
    } break;
    default:
        qWarning() << "Unsupported system type in Cooling loop" << mSystemSetup->systemType;
        break;
    }

    // turn off Y1, Y2 and G = 0 outside
}

void Scheme::HeatingLoop()
{
    TRACE_CHECK(true) << "Heating started " << mSystemSetup->systemType;

    // update configs and ...
    // s1 & s2 time threshold
    //    Y1, Y2, O/B G W1 W2 W3
    switch (mSystemSetup->systemType) {
    case AppSpecCPP::SystemType::HeatPump: // emergency as well?
    {
        TRACE << "HeatPump" << mCurrentTemperature << mSetPointTemperature;
        // get time threshold ETime
        if (mCurrentTemperature < mSetPointTemperature) {
            if (mCurrentTemperature < ET && mSystemSetup->heatPumpEmergency) {
                TRACE << "Emergency";
                EmergencyHeating();
            } else {
                TRACE << "Normal";
                internalPumpHeatingLoopStage1();
            }
        }
    } break;
    case AppSpecCPP::SystemType::Conventional:
    case AppSpecCPP::SystemType::HeatingOnly:
        TRACE << "Conventional" << mCurrentTemperature << mSetPointTemperature;
        if (mSetPointTemperature - mCurrentTemperature >= 1.9) {
            internalHeatingLoopStage1();
        }
        break;
    default:
        qWarning() << "Unsupported system type in Heating loop" << mSystemSetup->systemType;
        break;
    }

    // Turn off system (Y1, Y2, W1, W2,W3, G = 0) outside
}

void Scheme::VacationLoop()
{
    //TODO
    waitLoop();

    if ((mVacation.minimumTemperature - mCurrentTemperature) > 0.001) {
        setSetPointTemperature(mVacation.minimumTemperature);
        HeatingLoop();

    } else if ((mVacation.maximumTemperature - mCurrentTemperature) < 0.001) {
        setSetPointTemperature(mVacation.maximumTemperature);
        CoolingLoop();

    } else {
            switch (mSystemSetup->systemMode) {
            case AppSpecCPP::SystemMode::Cooling: {
                if (mSetPointTemperature  - mCurrentTemperature < 1) {
                    // mCurrentSysMode = AppSpecCPP::SystemMode::Cooling;
                    CoolingLoop();

                } else if (mSetPointTemperature  - mCurrentTemperature < 1.9) {
                    // mCurrentSysMode = AppSpecCPP::SystemMode::Off;
                    OffLoop();

                } else {
                    HeatingLoop();
                }

            } break;

            case AppSpecCPP::SystemMode::Heating: {
                if (mCurrentTemperature - mSetPointTemperature < 1) {
                    HeatingLoop();

                } else if (mCurrentTemperature - mSetPointTemperature < 1.9) {
                    OffLoop();

                }  else {
                    CoolingLoop();
                }

            } break;

            default: {
                if (mSetPointTemperature - mCurrentTemperature > 1.9) {
                    HeatingLoop();

                } else if (mCurrentTemperature - mSetPointTemperature > 1.9) {
                    CoolingLoop();

                }
            } break;
        }
    }


}

void Scheme::EmergencyLoop()
{
    //TODO
    waitLoop();
}

void Scheme::OffLoop()
{
    //TODO
    waitLoop();
}

void Scheme::internalCoolingLoopStage1(bool pumpHeat)
{
    if (pumpHeat) // how the system type setup get OB Orientatin
        mRelay->setOb_state(AppSpecCPP::Cooling);

    // sysDelay
    mRelay->coolingStage1();

    // 5 Sec
    emit changeBacklight(coolingColor);
    mTiming->s1uptime.restart();
    mTiming->s2Offtime.invalidate(); // s2 off time should be high value? or invalid?
    mTiming->uptime.restart();
    mTiming->s2hold = false;
    mTiming->alerts = false;
    sendRelays();

    while (mSetPointTemperature - mCurrentTemperature < 1) {
        TRACE << mCurrentTemperature << mSetPointTemperature << mRelay->relays().y2
              << mSystemSetup->coolStage << mTiming->s1uptime.elapsed()
              << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();
        if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mSystemSetup->coolStage == 2) {
            if (mCurrentTemperature - mSetPointTemperature >= 2.9
                || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 40 * 60000)) {
                if (!mTiming->s2Offtime.isValid() || mTiming->s2Offtime.elapsed() >= 2 * 60000) {
                    if (!internalCoolingLoopStage2()) {
                        break;
                    }
                }
            }
        } else {
            sendAlertIfNeeded();
        }

        if (stopWork)
            break;

        waitLoop();

        if (stopWork)
            break;
    }

    TRACE << mCurrentTemperature << mSetPointTemperature << "finished cooling" << stopWork;
}

bool Scheme::internalCoolingLoopStage2()
{
    TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s2hold;

    // turn on stage 2
    mRelay->coolingStage2();
    // 5 Sec
    emit changeBacklight(coolingColor);

    sendRelays();

    while (!stopWork) {
        TRACE << mTiming->s2hold << mSetPointTemperature << mCurrentTemperature;
        if (mTiming->s2hold) {
            if (mSetPointTemperature - mCurrentTemperature < 1) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (mCurrentTemperature - mSetPointTemperature > 1.9) {
                sendAlertIfNeeded();
            } else {
                break;
            }
        }

        if (stopWork)
            break;

        waitLoop();
    }

    TRACE << mCurrentTemperature << mSetPointTemperature << "finished stage 2" << stopWork;

    if (stopWork)
        return false;

    // to turn off stage 2
    mRelay->setAllOff();
    mRelay->coolingStage1();
    // 5 secs
    emit changeBacklight(coolingColor);
    mTiming->s1uptime.restart();
    mTiming->s2hold = true;
    mTiming->s2Offtime.restart();
    sendRelays();

    return true;
}

void Scheme::internalHeatingLoopStage1()
{
    // Sys delay
    mRelay->heatingStage1();
    // 5 secs
    emit changeBacklight(heatingColor);

    mTiming->s1uptime.restart();
    mTiming->uptime.restart();
    mTiming->s2hold = false;
    mTiming->s3hold = false;
    mTiming->alerts = false;
    // not sending?
    sendRelays();

    while (mCurrentTemperature - mSetPointTemperature < 1) {
        TRACE_CHECK(true) << mRelay->relays().w2 << mSystemSetup->heatStage
                          << mTiming->s1uptime.isValid() << mTiming->s1uptime.elapsed();
        if (mRelay->relays().w2 != STHERM::RelayMode::NoWire && mSystemSetup->heatStage >= 2) {
            if (mSetPointTemperature - mCurrentTemperature >= 2.9
                || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 10 * 60000)) {
                if (!internalHeatingLoopStage2())
                    break;
            }
        } else {
            sendAlertIfNeeded();
        }

        if (stopWork)
            break;

        // TODO should we wait for temp update before new loop?
        waitLoop();

        if (stopWork)
            break;
    }

    TRACE << mCurrentTemperature << mSetPointTemperature << "finished heating conventional"
          << stopWork;

    // will turn off all outside
}

bool Scheme::internalHeatingLoopStage2()
{
    TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s2hold;

    mRelay->heatingStage2();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s2uptime.restart();
    sendRelays();

    while (!stopWork) {
        TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s2hold;

        if (mTiming->s2hold) {
            if (mCurrentTemperature - mSetPointTemperature < 1) {
                if ((mRelay->relays().w3 == STHERM::RelayMode::NoWire || mSystemSetup->coolStage < 3)
                    || (mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() < 10 * 60000)) {
                    sendAlertIfNeeded();
                } else {
                    // stage 3
                    if (!internalHeatingLoopStage3())
                        return false;
                }
            } else {
                // all will be turned off outside
                return false;
            }
        } else {
            if (mSetPointTemperature - mCurrentTemperature < 8
                || (mRelay->relays().w3 == STHERM::RelayMode::NoWire || mSystemSetup->coolStage < 3)) {
                if (mSetPointTemperature - mCurrentTemperature < 1.9) {
                    break;
                } else {
                    sendAlertIfNeeded();
                }
            } else {
                if (mSetPointTemperature - mCurrentTemperature < 5.9
                    && (mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() < 10 * 60000)) {
                    sendAlertIfNeeded();
                } else {
                    //stage 3
                    if (!internalHeatingLoopStage3())
                        return false;
                }
            }
        }

        if (stopWork)
            break;

        waitLoop();
    }

    TRACE << mCurrentTemperature << mSetPointTemperature << "finished stage 2 heat" << stopWork;

    if (stopWork)
        return false;

    //turn of stage 2
    mRelay->setAllOff();
    mRelay->heatingStage1();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s1uptime.restart();
    mTiming->s2hold = true;
    sendRelays();

    return true;
}

bool Scheme::internalHeatingLoopStage3()
{
    TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s3hold;

    mRelay->heatingStage3();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s2hold = false;
    sendRelays();

    while (!stopWork) {
        TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s3hold;
        if (mTiming->s3hold) {
            if (mCurrentTemperature - mSetPointTemperature < 1) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (mSetPointTemperature - mCurrentTemperature < 4.9) {
                break;
            } else {
                sendAlertIfNeeded();
            }
        }
        if (stopWork)
            break;

        waitLoop();
    }
    TRACE << mCurrentTemperature << mSetPointTemperature << "finished stage 3" << stopWork;

    if (stopWork)
        return false;

    //turn of stage 3
    mRelay->setAllOff();
    mRelay->heatingStage2();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s1uptime.restart();
    mTiming->s2uptime.restart();
    mTiming->s3hold = true;
    sendRelays();
    return true;
}

void Scheme::internalPumpHeatingLoopStage1()
{
    if (mSetPointTemperature - mCurrentTemperature >= 3) {
        mRelay->setOb_state(AppSpecCPP::Heating);

        // sysDelay
        mRelay->heatingStage1();

        // 5 Sec
        emit changeBacklight(heatingColor);
        mTiming->s1uptime.restart();
        mTiming->s2Offtime.invalidate(); // s2 off time should be high value? or invalid?
        mTiming->uptime.restart();
        mTiming->s2hold = false;
        mTiming->alerts = false;
        sendRelays();

        while (mCurrentTemperature - mSetPointTemperature < 1.9) {
            TRACE << mCurrentTemperature << mSetPointTemperature << mRelay->relays().y2
                  << mSystemSetup->heatStage << mTiming->s1uptime.elapsed()
                  << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();

            if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mSystemSetup->heatStage >= 2) {
                if (mSetPointTemperature - mCurrentTemperature >= 2.9
                    || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 40 * 60000)) {
                    if (!mTiming->s2Offtime.isValid() || mTiming->s2Offtime.elapsed() >= 2 * 60000) {
                        if (!internalPumpHeatingLoopStage2()) {
                            break;
                        }
                    }
                }
            } else {
                sendAlertIfNeeded();
                // wait for temperature update?
            }

            if (stopWork)
                break;

            waitLoop();

            if (stopWork)
                break;
        }
    }

    TRACE << mCurrentTemperature << mSetPointTemperature << "finished pump heat" << stopWork;
}

bool Scheme::internalPumpHeatingLoopStage2()
{
    TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s2hold;
    // turn on stage 2
    mRelay->heatingStage2();
    // 5 Sec
    emit changeBacklight(coolingColor);
    sendRelays();

    while (!stopWork) {
        TRACE << mCurrentTemperature << mSetPointTemperature << mTiming->s2hold;

        if (mTiming->s2hold) {
            if (mCurrentTemperature - mSetPointTemperature < 1) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (mSetPointTemperature - mCurrentTemperature > 1.9) {
                sendAlertIfNeeded();
            } else {
                break;
            }
        }

        if (stopWork)
            break;

        waitLoop();
    }
    TRACE << mCurrentTemperature << mSetPointTemperature << "finished stage 2 pump" << stopWork;

    if (stopWork)
        return false;

    // to turn off stage 2
    mRelay->setAllOff();
    mRelay->heatingStage1();
    // 5 secs
    emit changeBacklight(coolingColor);
    mTiming->s1uptime.restart();
    mTiming->s2hold = true;
    mTiming->s2Offtime.restart();
    sendRelays();

    return true;
}

void Scheme::EmergencyHeating()
{
    mRelay->setAllOff();
    mRelay->emergencyHeating1();

    // 5 Sec
    emit changeBacklight(emergencyColor, emergencyColorS);
    sendRelays();

    bool stage2 = false;
    while (mCurrentTemperature < HPT) {
        if (!stage2 && mCurrentTemperature < ET - ET_STAGE2) {
            mRelay->emergencyHeating2();
            sendRelays();
            stage2 = true;
        }
    }

    emit changeBacklight();
    mRelay->turnOffEmergencyHeating();
    sendRelays();

    internalPumpHeatingLoopStage1();
}

void Scheme::sendAlertIfNeeded()
{
    // Generate Alert
    if (!mTiming->alerts
        && (mTiming->uptime.isValid() && mTiming->uptime.elapsed() >= 120 * 60000)) {
        TRACE;
        emit alert();
        mTiming->alerts = true;
    }
}

void Scheme::sendRelays()
{
    if (stopWork)
        return;

    TRACE;

    // Update relays
    emit updateRelays(mRelay->relays());
    this->msleep(5000);

    TRACE;
}

int Scheme::waitLoop(int timeout)
{
    QEventLoop loop;
    // connect signal for handling stopWork
    connect(this, &Scheme::currentTemperatureChanged, &loop, [&loop]() {
        loop.exit(ChangeType::CurrentTemperature);
    });

    connect(this, &Scheme::setTemperatureChanged, &loop, [&loop]() {
        loop.exit(ChangeType::SetTemperature);
    });

    connect(this, &Scheme::stopWorkRequested, &loop, [&loop]() {
        loop.exit(ChangeType::Mode);
    });

    if (timeout == 0) {
        return 0;
    } else if (timeout > 0) {
        // quit will exit with, same as exit(ChangeType::CurrentTemperature)
        QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
    }

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
    double currentTemp = 32.0 + mainData.value("temperature").toDouble(&isOk) * 9 / 5;

    if (isOk && currentTemp != mCurrentTemperature) {
        mCurrentTemperature = currentTemp;

        emit currentTemperatureChanged();
    }

    double currentHumidity =mainData.value("humidity").toDouble(&isOk);

    if (isOk) {
        setCurrentHumidity(currentHumidity);
    }
}

//TODO
void Scheme::updateVacationState()
{
    if (stopWork)
       return;

    if (mRealSysMode != AppSpecCPP::SystemMode::Vacation)
       return; // we can also assert as this should not happen

    TRACE << "mCurrentSysMode " << mCurrentSysMode;
    AppSpecCPP::SystemMode realSysMode = AppSpecCPP::SystemMode::Off;

    if (mCurrentSysMode == AppSpecCPP::SystemMode::Cooling) {
        if (mCurrentTemperature > mSetPointTemperature - STAGE1_OFF_RANGE) { // before stage 1 off
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        } else if (mCurrentTemperature > mSetPointTemperature - STAGE1_ON_RANGE) { // before stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Off;
        } else {  // stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Heating;
        }
    } else if (mCurrentSysMode == AppSpecCPP::SystemMode::Heating) {
        if (mCurrentTemperature < mSetPointTemperature + STAGE1_OFF_RANGE) { // before stage 1 off
            realSysMode = AppSpecCPP::SystemMode::Heating;
        } else if (mCurrentTemperature < mSetPointTemperature + STAGE1_ON_RANGE) { // before stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Off;
        } else {  // stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        }
    } else { // OFF
        if (mCurrentTemperature < mSetPointTemperature - STAGE1_ON_RANGE) {
            realSysMode = AppSpecCPP::SystemMode::Heating;
        } else if (mCurrentTemperature > mSetPointTemperature + STAGE1_ON_RANGE) {
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        } else {
            // what should we do here?
        }
    }

    if (mVacation.minimumTemperature > mCurrentTemperature) {
        realSysMode = AppSpecCPP::SystemMode::Heating;
        //        range = temperature - $current_state['min_temp'];
    } else if (mVacation.maximumTemperature < mCurrentTemperature) {
        realSysMode = AppSpecCPP::SystemMode::Cooling;
        //        range = temperature - current_state['max_temp'];
    }

    updateHumifiresState();

    // Update current system mode
    mCurrentSysMode = realSysMode;
    mRealSysMode = realSysMode;
}

// TODO
void Scheme::updateHumifiresState()
{
    if (stopWork)
        return;

    TRACE << "HumidifierId " << mHumidifierId << mSystemSetup;

    if (!mSystemSetup || mHumidifierId == 3)
        return;

    if (mRealSysMode == AppSpecCPP::Vacation) {
        qDebug() << Q_FUNC_INFO << __LINE__ <<mRealSysMode;
        if (mHumidifierId == 1) {
            mRelay->setHumidifierState(mCurrentHumidity < mVacation.minimumHumidity);

        } else if (mHumidifierId == 2) {
            mRelay->setDehumidifierState(mCurrentHumidity > mVacation.maximumHumidity);
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
                mRelay->setHumidifierState(true);

            else if (mCurrentHumidity >= max_hum)    // off
                mRelay->setHumidifierState(false);

        } else if (mHumidifierId == 2) { // dehumidifier
            if (mCurrentHumidity > mSetPointHimidity)
                mRelay->setDehumidifierState(true);
            else if (mCurrentHumidity <= min_hum)
                mRelay->setDehumidifierState(false);
        }
    }
}

void Scheme::setVacation(const STHERM::Vacation &newVacation)
{
    mVacation = newVacation;
}

void Scheme::setFanWorkPerHour(int newFanWPH)
{
    if (mFanWPH == newFanWPH)
        return;

    mFanWPH = newFanWPH;

    int fanWork = QDateTime::currentSecsSinceEpoch() - mTiming->fan_time.toSecsSinceEpoch() - mFanWPH - 1;
    mRelay->fanWorkTime(mFanWPH, fanWork);
}

void Scheme::setSystemSetup(SystemSetup *systemSetup)
{
    TRACE << systemSetup << mSystemSetup;
    if (!systemSetup || mSystemSetup == systemSetup)
        return;

    if (mSystemSetup) {
        mSystemSetup->disconnect(this);
    }

    mSystemSetup = systemSetup;

    connect(mSystemSetup, &SystemSetup::systemModeChanged, this, [this] {
        TRACE<< "systemModeChanged: "<< mSystemSetup->systemMode;

        restartWork();
    });

    connect(mSystemSetup, &SystemSetup::isVacationChanged, this, [this] {
        TRACE<< "isVacationChanged: "<< mSystemSetup->isVacation;

        restartWork();
    });

    connect(mSystemSetup, &SystemSetup::systemTypeChanged, this, [this] {
        TRACE<< "systemTypeChanged: "<< mSystemSetup->systemType;

        restartWork();
    });

    // these parameters will be used in control loop, if any condition locked to these update here
    connect(mSystemSetup, &SystemSetup::heatPumpOBStateChanged, this, [this] {
        if (mSystemSetup->systemType == AppSpecCPP::SystemType::HeatPump)
            TRACE << "heatPumpOBStateChanged: " << mSystemSetup->heatPumpOBState
                  << mSystemSetup->systemType;
    });
    connect(mSystemSetup, &SystemSetup::coolStageChanged, this, [this] {
        if (mSystemSetup->systemType == AppSpecCPP::SystemType::CoolingOnly)
            TRACE << "coolStageChanged: " << mSystemSetup->coolStage;
    });
    connect(mSystemSetup, &SystemSetup::heatStageChanged, this, [this] {
        if (mSystemSetup->systemType == AppSpecCPP::SystemType::HeatingOnly)
            TRACE << "heatStageChanged: " << mSystemSetup->heatStage;
    });
    connect(mSystemSetup, &SystemSetup::systemRunDelayChanged, this, [this] {
        TRACE << "systemRunDelayChanged: " << mSystemSetup->systemRunDelay
              << mSystemSetup->systemType;
    });
    connect(mSystemSetup, &SystemSetup::heatPumpEmergencyChanged, this, [this] {
        if (mSystemSetup->systemType == AppSpecCPP::SystemType::HeatPump)
            TRACE << "heatPumpEmergencyChanged: " << mSystemSetup->heatPumpEmergency;
    });
}

void Scheme::setHumidifierId(const int &humidifierId)
{
    if (mHumidifierId != humidifierId)
        return;

    mHumidifierId = humidifierId;
    updateHumifiresState();

}

AppSpecCPP::SystemMode Scheme::getCurrentSysMode() const
{
    return mCurrentSysMode;
}

void Scheme::setCurrentSysMode(AppSpecCPP::SystemMode newSysMode)
{
    if (mCurrentSysMode == newSysMode)
        return;

    mCurrentSysMode = newSysMode;
}
