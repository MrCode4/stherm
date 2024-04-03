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
const double RELAYS_WAIT_MS          = 500;

// Status backlight color
const QVariantList coolingColor      = QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE, "true"};
const QVariantList heatingColor      = QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE,  "true"};
const QVariantList emergencyColor    = QVariantList{255, 0, 0, STHERM::LedEffect::LED_BLINK,  "true"};
const QVariantList emergencyColorS   = QVariantList{255, 0, 0, STHERM::LedEffect::LED_STABLE, "true"};

Scheme::Scheme(DeviceAPI* deviceAPI, QObject *parent) :
    mDeviceAPI(deviceAPI),
    mRestarting (false),
    QThread (parent)
{
    stopWork = true;
    debugMode = RELAYS_WAIT_MS != 0;

    mTiming = mDeviceAPI->timing();
    mRelay  = Relay::instance();

    mCurrentSysMode = AppSpecCPP::SystemMode::Auto;

    mFanHourTimer.setTimerType(Qt::PreciseTimer);
    mFanHourTimer.setSingleShot(false);
    mFanHourTimer.connect(&mFanHourTimer, &QTimer::timeout, this, [=]() {
        // Fan on when 1 hour finished and fan mode in on
        if (mFanMode == AppSpecCPP::FMOn) {
            fanWork(true);
        }
    });

    mFanWPHTimer.setTimerType(Qt::PreciseTimer);
    mFanWPHTimer.setSingleShot(true);
    mFanWPHTimer.connect(&mFanWPHTimer, &QTimer::timeout, this, [=]() {
        fanWork(false);
    });

    mUpdatingTimer.setTimerType(Qt::PreciseTimer);
    mUpdatingTimer.setSingleShot(true);
    mUpdatingTimer.connect(&mUpdatingTimer, &QTimer::timeout, this, [=]() {
        mRestarting = false;
    });

    mLogTimer.setInterval(30000);
    mLogTimer.connect(&mLogTimer, &QTimer::timeout, this, [=]() {

        LOG_CHECK(isRunning()) << "Scheme Running with these parameters: -------------------------------";
        LOG_CHECK(isRunning() && mSystemSetup) << "systemMode: " << mSystemSetup->systemMode << "systemType: " << mSystemSetup->systemType;
        LOG_CHECK(isRunning() && mSystemSetup) << "systemRunDelay: " << mSystemSetup->systemRunDelay << "isVacation: " << mSystemSetup->isVacation;
        LOG_CHECK(isRunning() && mSystemSetup) << "heatStage: "  << mSystemSetup->heatStage << "coolStage: " << mSystemSetup->coolStage;
        LOG_CHECK(isRunning() && mSystemSetup) << "heatPumpEmergency: "  << mSystemSetup->heatPumpEmergency << "heatPumpOBState: " << (mSystemSetup->heatPumpOBState == 0 ? "cooling" : "heating");
        LOG_CHECK(isRunning() && mSystemSetup) << "systemAccessories (wire, typ): " << mSystemSetup->systemAccessories->property("accessoriesWireType") <<  mSystemSetup->systemAccessories->property("accessoriesType");
        LOG_CHECK(isRunning() && mRelay) << "getOb_state: "  << mRelay->getOb_state() << "relays: " << mRelay->relays().printStr();
        LOG_CHECK(isRunning() && mSchedule) << "Schedule : " << mSchedule->name;
        LOG_CHECK(isRunning()) << "Timing S1UT: " << mTiming->s1uptime.elapsed() << "S2UT: " << mTiming->s2uptime.elapsed();
        LOG_CHECK(isRunning()) << "S2OT: " << mTiming->s2Offtime.elapsed() << "uptime: " << mTiming->uptime.elapsed();
        LOG_CHECK(isRunning()) << "alert:" << mTiming->alerts << "s3hold:" << mTiming->s3hold << "s2hold:" << mTiming->s2hold;
        LOG_CHECK(isRunning() && mFanHourTimer.isActive()) << "fan on 1 hour stage finishes in " << mFanHourTimer.remainingTime() / 60000 << "minutes";
        LOG_CHECK(isRunning() && mFanWPHTimer.isActive()) << "fan on stage finishes in " << mFanWPHTimer.remainingTime() / 60000 << "minutes";
        LOG_CHECK(isRunning()) << "Current Temperature : " << effectiveCurrentTemperature() << "Effective Set Temperature: " << effectiveTemperature();
        LOG_CHECK(isRunning() && false) << "Current Humidity : " << effectiveCurrentHumidity() << "Effective Set Humidity: " << effectiveHumidity();
        LOG_CHECK(!isRunning()) << "-----------------------------Scheme is stopped ---------------------";
        LOG_CHECK(isRunning())  << "-----------------------------Scheme Log Ended  ---------------------";
    });
}

void Scheme::stop()
{
    TRACE << "stopping HVAC" ;

    if (mLogTimer.isActive())
        mLogTimer.stop();

    if (mUpdatingTimer.isActive())
        mUpdatingTimer.stop();

    if (mFanHourTimer.isActive())
        mFanHourTimer.stop();

    if (mFanWPHTimer.isActive())
        mFanWPHTimer.stop();

    stopWork = true;

    // Stop worker.
    terminate();
    TRACE << "terminated HVAC" ;
    wait();

    TRACE << "stopped HVAC" ;
}

Scheme::~Scheme()
{
    stop();
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
                mLogTimer.start();
                this->start();
            },
            Qt::SingleShotConnection);

        stopWork = true;
        emit stopWorkRequested();
        this->wait(QDeadlineTimer(1000, Qt::PreciseTimer));

    } else {
        TRACE << "started HVAC";
        stopWork = false;
        mLogTimer.start();
        this->start();
    }
}

void Scheme::setSetPointTemperature(double newSetPointTemperature)
{
    if (qAbs(mSetPointTemperature - newSetPointTemperature) < 0.001)
        return;

    mSetPointTemperature = newSetPointTemperature;

    TRACE << "mSetPointTemperature changed";
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

        // Vacation has a higher priority compared to other processes.
        if (mSystemSetup->isVacation) {
            VacationLoop();

        } else if (mSchedule) { // Schedule moves the app to auto mode and use the schedule property (temperature/humidity)
            AutoModeLoop();

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

        if (!mRestarting) {
            mRelay->setAllOff();

            //        int fanWork = QDateTime::currentSecsSinceEpoch() - mTiming->fan_time.toSecsSinceEpoch() - mFanWPH - 1;
            //        mRelay->fanWorkTime(mFanWPH, fanWork);

            sendRelays();
        }

        if (stopWork)
            break;

        // all should be off! we can assert here
        waitLoop(RELAYS_WAIT_MS);
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
    if (mCurrentTemperature > effectiveTemperature()) {
        CoolingLoop();
    } else if (mCurrentTemperature < effectiveTemperature()) {
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
        TRACE << heatPump << mCurrentTemperature << effectiveTemperature();
        if (mCurrentTemperature - effectiveTemperature() >= 1.9) {
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
        TRACE << "HeatPump" << mCurrentTemperature << effectiveTemperature();
        // get time threshold ETime
        if (mCurrentTemperature < effectiveTemperature()) {
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
        TRACE << "Conventional" << mCurrentTemperature << effectiveTemperature();
        if (effectiveTemperature() - mCurrentTemperature >= 1.9) {
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
        HeatingLoop();

    } else if ((mVacation.maximumTemperature - mCurrentTemperature) < 0.001) {
        CoolingLoop();

    } else {
            switch (mSystemSetup->systemMode) {
            case AppSpecCPP::SystemMode::Cooling: {
                if (effectiveTemperature() - mCurrentTemperature < 1) {
                    // mCurrentSysMode = AppSpecCPP::SystemMode::Cooling;
                    CoolingLoop();

                } else if (effectiveTemperature() - mCurrentTemperature < 1.9) {
                    // mCurrentSysMode = AppSpecCPP::SystemMode::Off;
                    OffLoop();

                } else {
                    HeatingLoop();
                }

            } break;

            case AppSpecCPP::SystemMode::Heating: {
                if (mCurrentTemperature - effectiveTemperature() < 1) {
                    HeatingLoop();

                } else if (mCurrentTemperature - effectiveTemperature() < 1.9) {
                    OffLoop();

                }  else {
                    CoolingLoop();
                }

            } break;

            default: {
                if (effectiveTemperature() - mCurrentTemperature > 1.9) {
                    HeatingLoop();

                } else if (mCurrentTemperature - effectiveTemperature() > 1.9) {
                    CoolingLoop();

                }
            } break;
        }
    }

    updateHumifiresState();
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

void Scheme::updateOBState(AppSpecCPP::SystemMode newOb_state)
{
    // we should check if it is changed or not!
    if (mRelay->setOb_state(newOb_state))
    {
        bool relaysChanged = true;
        sendRelays();
        if (relaysChanged && !stopWork){
            // sysDelay
            waitLoop(mSystemSetup->systemRunDelay * 60000);
        }

        // Send current system mode into deviceController when is needed
        emit currentSystemModeChanged(newOb_state);
    }
}

void Scheme::internalCoolingLoopStage1(bool pumpHeat)
{
    if (pumpHeat) // how the system type setup get OB Orientatin
    {
        updateOBState(AppSpecCPP::Cooling);
    }

    if (stopWork)
        return;

    mRelay->coolingStage1();

    // 5 Sec
    emit changeBacklight(coolingColor);
    mTiming->s1uptime.restart();
    mTiming->s2Offtime.invalidate(); // s2 off time should be high value? or invalid?
    mTiming->uptime.restart();
    mTiming->s2hold = false;
    mTiming->alerts = false;
    sendRelays();
    waitLoop(RELAYS_WAIT_MS);

    while (effectiveTemperature() - mCurrentTemperature < 1) {
        TRACE << mCurrentTemperature << effectiveTemperature() << mRelay->relays().y2
              << mSystemSetup->coolStage << mTiming->s1uptime.elapsed()
              << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();
        if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mSystemSetup->coolStage == 2) {
            if (mCurrentTemperature - effectiveTemperature() >= 2.9
                || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 40 * 60000)) {
                if (!mTiming->s2Offtime.isValid() || mTiming->s2Offtime.elapsed() >= 2 * 60000) {
                    if (!internalCoolingLoopStage2()) {
                        break;
                    }
                }
            }
        } else {
            sendAlertIfNeeded();
            if (stopWork)
                break;
            waitLoop();
        }

        if (stopWork)
            break;

        waitLoop(RELAYS_WAIT_MS);

        if (stopWork)
            break;
    }

    TRACE << mCurrentTemperature << effectiveTemperature() << "finished cooling" << stopWork;
}

bool Scheme::internalCoolingLoopStage2()
{
    TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s2hold;

    // turn on stage 2
    mRelay->coolingStage2();
    // 5 Sec
    emit changeBacklight(coolingColor);

    sendRelays();

    while (!stopWork) {
        TRACE << mTiming->s2hold << effectiveTemperature() << mCurrentTemperature;
        if (mTiming->s2hold) {
            if (effectiveTemperature() - mCurrentTemperature < 1) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (mCurrentTemperature - effectiveTemperature() > 1.9) {
                sendAlertIfNeeded();
            } else {
                break;
            }
        }

        if (stopWork)
            break;

        waitLoop();
    }

    TRACE << mCurrentTemperature << effectiveTemperature() << "finished stage 2" << stopWork;

    if (stopWork)
        return false;

    if (!mRestarting) {
        // to turn off stage 2
        mRelay->setAllOff();
        mRelay->coolingStage1();
        // 5 secs
        emit changeBacklight(coolingColor);
        mTiming->s1uptime.restart();
        mTiming->s2hold = true;
        mTiming->s2Offtime.restart();
        sendRelays();
    }

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
    waitLoop(RELAYS_WAIT_MS);

    while (mCurrentTemperature - effectiveTemperature() < 1) {
        TRACE_CHECK(true) << mRelay->relays().w2 << mSystemSetup->heatStage
                          << mTiming->s1uptime.isValid() << mTiming->s1uptime.elapsed();
        if (mRelay->relays().w2 != STHERM::RelayMode::NoWire && mSystemSetup->heatStage >= 2) {
            if (effectiveTemperature() - mCurrentTemperature >= 2.9
                || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 10 * 60000)) {
                if (!internalHeatingLoopStage2())
                    break;
            }
        } else {
            sendAlertIfNeeded();
            if (stopWork)
                break;
            waitLoop();
        }

        if (stopWork)
            break;

        // TODO should we wait for temp update before new loop?
        waitLoop(RELAYS_WAIT_MS);

        if (stopWork)
            break;
    }

    TRACE << mCurrentTemperature << effectiveTemperature() << "finished heating conventional"
          << stopWork;

    // will turn off all outside
}

bool Scheme::internalHeatingLoopStage2()
{
    TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s2hold;

    mRelay->heatingStage2();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s2uptime.restart();
    sendRelays();
    waitLoop(RELAYS_WAIT_MS);

    while (!stopWork) {
        TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s2hold;

        if (mTiming->s2hold) {
            if (mCurrentTemperature - effectiveTemperature() < 1) {
                if ((mRelay->relays().w3 == STHERM::RelayMode::NoWire || mSystemSetup->coolStage < 3)
                    || (mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() < 10 * 60000)) {
                    sendAlertIfNeeded();
                    if (stopWork)
                        break;
                    waitLoop();
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
            if (effectiveTemperature() - mCurrentTemperature < 8
                || (mRelay->relays().w3 == STHERM::RelayMode::NoWire || mSystemSetup->coolStage < 3)) {
                if (effectiveTemperature() - mCurrentTemperature < 1.9) {
                    break;
                } else {
                    sendAlertIfNeeded();
                }
                if (stopWork)
                    break;
                waitLoop();
            } else {
                if (effectiveTemperature() - mCurrentTemperature < 5.9
                    && (mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() < 10 * 60000)) {
                    sendAlertIfNeeded();
                    if (stopWork)
                        break;
                    waitLoop();
                } else {
                    //stage 3
                    if (!internalHeatingLoopStage3())
                        return false;
                }
            }
        }

        if (stopWork)
            break;

        waitLoop(RELAYS_WAIT_MS);
    }

    TRACE << mCurrentTemperature << effectiveTemperature() << "finished stage 2 heat" << stopWork;

    if (stopWork)
        return false;

    if (!mRestarting) {
        //turn of stage 2
        mRelay->setAllOff();
        mRelay->heatingStage1();
        // 5 secs
        emit changeBacklight(heatingColor);
        mTiming->s1uptime.restart();
        mTiming->s2hold = true;
        sendRelays();
    }

    return true;
}

bool Scheme::internalHeatingLoopStage3()
{
    TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s3hold;

    mRelay->heatingStage3();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s2hold = false;
    sendRelays();

    while (!stopWork) {
        TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s3hold;
        if (mTiming->s3hold) {
            if (mCurrentTemperature - effectiveTemperature() < 1) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (effectiveTemperature() - mCurrentTemperature < 4.9) {
                break;
            } else {
                sendAlertIfNeeded();
            }
        }
        if (stopWork)
            break;

        waitLoop();
    }
    TRACE << mCurrentTemperature << effectiveTemperature() << "finished stage 3" << stopWork;

    if (stopWork)
        return false;

    if (!mRestarting) {
        //turn of stage 3
        mRelay->setAllOff();
        mRelay->heatingStage2();
        // 5 secs
        emit changeBacklight(heatingColor);
        mTiming->s1uptime.restart();
        mTiming->s2uptime.restart();
        mTiming->s3hold = true;
        sendRelays();
    }

    return true;
}

void Scheme::internalPumpHeatingLoopStage1()
{
    if (effectiveTemperature() - mCurrentTemperature >= 3) {
        updateOBState(AppSpecCPP::Heating);

        if (stopWork)
            return;

        mRelay->heatingStage1(true);

        // 5 Sec
        emit changeBacklight(heatingColor);
        mTiming->s1uptime.restart();
        mTiming->s2Offtime.invalidate(); // s2 off time should be high value? or invalid?
        mTiming->uptime.restart();
        mTiming->s2hold = false;
        mTiming->alerts = false;
        sendRelays();
        waitLoop(RELAYS_WAIT_MS);

        while (mCurrentTemperature - effectiveTemperature() < 1.9) {
            TRACE << mCurrentTemperature << effectiveTemperature() << mRelay->relays().y2
                  << mSystemSetup->heatStage << mTiming->s1uptime.elapsed()
                  << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();

            if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mSystemSetup->heatStage >= 2) {
                if (effectiveTemperature() - mCurrentTemperature >= 2.9
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
                if (stopWork)
                    break;
                waitLoop();
            }

            if (stopWork)
                break;

            waitLoop(RELAYS_WAIT_MS);

            if (stopWork)
                break;
        }
    }

    TRACE << mCurrentTemperature << effectiveTemperature() << "finished pump heat" << stopWork;
}

bool Scheme::internalPumpHeatingLoopStage2()
{
    TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s2hold;
    // turn on stage 2
    mRelay->heatingStage2(true);
    // 5 Sec
    emit changeBacklight(heatingColor);
    sendRelays();

    while (!stopWork) {
        TRACE << mCurrentTemperature << effectiveTemperature() << mTiming->s2hold;

        if (mTiming->s2hold) {
            if (mCurrentTemperature - effectiveTemperature() < 1) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (effectiveTemperature() - mCurrentTemperature > 1.9) {
                sendAlertIfNeeded();
            } else {
                break;
            }
        }

        if (stopWork)
            break;

        waitLoop();
    }
    TRACE << mCurrentTemperature << effectiveTemperature() << "finished stage 2 pump" << stopWork;

    if (stopWork)
        return false;

    if (!mRestarting) {
        // to turn off stage 2
        mRelay->setAllOff();
        mRelay->heatingStage1(true);
        // 5 secs
        emit changeBacklight(heatingColor);
        mTiming->s1uptime.restart();
        mTiming->s2hold = true;
        mTiming->s2Offtime.restart();
        sendRelays();
    }

    return true;
}

void Scheme::EmergencyHeating()
{
    mRelay->setAllOff();
    mRelay->emergencyHeating1();

    // 5 Sec
    emit changeBacklight(emergencyColor, emergencyColorS);
    sendRelays();
    waitLoop(RELAYS_WAIT_MS);

    bool stage2 = false;
    while (mCurrentTemperature < HPT) {
        if (!stage2 && mCurrentTemperature < ET - ET_STAGE2) {
            mRelay->emergencyHeating2();
            sendRelays();
            waitLoop(RELAYS_WAIT_MS);
            stage2 = true;
        }
    }

    emit changeBacklight();
    mRelay->turnOffEmergencyHeating();
    sendRelays();
    waitLoop(RELAYS_WAIT_MS);

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

    auto relaysConfig = mRelay->relays();

    if (debugMode) {
        if (lastConfigs == relaysConfig) {
            TRACE_CHECK(false) << "no change";
            return;
        }
        auto steps = lastConfigs.changeStepsSorted(relaysConfig);
        for (int var = 0; var < steps.size(); ++var) {
            auto step = steps.at(var);
            TRACE << step.first.c_str() << step.second;
            if (step.first == "o/b"){
                lastConfigs.o_b = relaysConfig.o_b;
                TRACE << relaysConfig.o_b;
            }
            else if (step.first == "g"){
                lastConfigs.g = relaysConfig.g;
                TRACE << relaysConfig.g;
            }
            else if (step.first == "y1"){
                lastConfigs.y1 = relaysConfig.y1;
                TRACE << relaysConfig.y1;
            }
            else if (step.first == "y2"){
                lastConfigs.y2 = relaysConfig.y2;
                TRACE << relaysConfig.y2;
            }
            else if (step.first == "w1"){
                lastConfigs.w1 = relaysConfig.w1;
                TRACE << relaysConfig.w1;
            }
            else if (step.first == "w2"){
                lastConfigs.w2 = relaysConfig.w2;
                TRACE << relaysConfig.w2;
            }
            else if (step.first == "w3"){
                lastConfigs.w3 = relaysConfig.w3;
                TRACE << relaysConfig.w3;
            }

            // Update relays
            if (step.second != 0) {
                emit updateRelays(lastConfigs);
                waitLoop(RELAYS_WAIT_MS);
            }
        }
    } else { // Update relays
        emit updateRelays(relaysConfig);
        lastConfigs = relaysConfig;
    }

    TRACE_CHECK(false) << "finished";
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
    double tc = mainData.value("temperature").toDouble(&isOk);
    double currentTemp = 32.0 + tc * 9 / 5;

    if (isOk && currentTemp != mCurrentTemperature) {
        mCurrentTemperature = currentTemp;

        emit currentTemperatureChanged();
    }

    double currentHumidity = mainData.value("humidity").toDouble(&isOk);

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
        if (mCurrentTemperature > effectiveTemperature() - STAGE1_OFF_RANGE) { // before stage 1 off
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        } else if (mCurrentTemperature > effectiveTemperature() - STAGE1_ON_RANGE) { // before stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Off;
        } else {  // stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Heating;
        }
    } else if (mCurrentSysMode == AppSpecCPP::SystemMode::Heating) {
        if (mCurrentTemperature < effectiveTemperature() + STAGE1_OFF_RANGE) { // before stage 1 off
            realSysMode = AppSpecCPP::SystemMode::Heating;
        } else if (mCurrentTemperature < effectiveTemperature() + STAGE1_ON_RANGE) { // before stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Off;
        } else {  // stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        }
    } else { // OFF
        if (mCurrentTemperature < effectiveTemperature() - STAGE1_ON_RANGE) {
            realSysMode = AppSpecCPP::SystemMode::Heating;
        } else if (mCurrentTemperature > effectiveTemperature() + STAGE1_ON_RANGE) {
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

    if (mSystemSetup->isVacation) {
        qDebug() << Q_FUNC_INFO << __LINE__ << mRealSysMode;
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

void Scheme::setSchedule(ScheduleCPP *newSchedule)
{
    if (mSchedule == newSchedule)
        return;

    mSchedule = newSchedule;
}

void Scheme::moveToUpdatingMode()
{
    mRestarting = true;

    // If the system fails to restart within 10 seconds, it returns to normal operation.
    mUpdatingTimer.start(10000);
}

double Scheme::effectiveTemperature()
{
    double effTemperature = mSetPointTemperature;

    if (mSystemSetup->isVacation) {
        if ((mVacation.minimumTemperature - mCurrentTemperature) > 0.001) {
            effTemperature = mVacation.minimumTemperature;

        } else if ((mVacation.maximumTemperature - mCurrentTemperature) < 0.001) {
            effTemperature = mVacation.maximumTemperature;
        }

    } else if (mSchedule) {
        effTemperature = mSchedule->temprature;

    }

    // Convert to F
    return (32.0 + effTemperature * 9 / 5);
}

double Scheme::effectiveCurrentTemperature()
{
    return mCurrentTemperature;
}

double Scheme::effectiveHumidity()
{
    return mSetPointHimidity;
}

double Scheme::effectiveCurrentHumidity()
{
    return mCurrentHumidity;
}

void Scheme::setVacation(const STHERM::Vacation &newVacation)
{
    mVacation = newVacation;
}

void Scheme::setFan(AppSpecCPP::FanMode fanMode, int newFanWPH)
{
    mFanMode = fanMode;
    mFanWPH = newFanWPH;

    if (mFanMode == AppSpecCPP::FMOn && newFanWPH > 0) {
        mFanHourTimer.start(1 * 60 * 60 * 1000);
        fanWork(true);

    } else {
        mFanHourTimer.stop();
        fanWork(false);

    }

}
void Scheme::fanWork(bool isOn) {

    emit fanWorkChanged(isOn);
    if (isOn) {
        mFanWPHTimer.start(mFanWPH * 60 * 1000);

    } else {
        // Move to auto mode
        mFanWPHTimer.stop();
    }

    mRelay->setFanMode(isOn);

    sendRelays();
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

    // ob_state_ initial value?
    mRelay->setOb_on_state(mSystemSetup->heatPumpOBState == 0 ? AppSpecCPP::Cooling
                                                              : AppSpecCPP::Heating);

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
        mRelay->setOb_on_state(mSystemSetup->heatPumpOBState == 0 ? AppSpecCPP::Cooling
                                                                  : AppSpecCPP::Heating);
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
