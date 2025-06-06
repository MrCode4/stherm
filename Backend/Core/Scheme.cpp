#include "Scheme.h"

#include <QColor>
#include <QEventLoop>
#include <QTimer>

#include "LogCategoires.h"
#include "SchemeDataProvider.h"

const double ET                      = 40;                 // 40 F or 4.5 C
const double ET_STAGE2               = 3.5;
const double HPT                     = 60;                // 15 C
const double HUM_MAX                 = 70;
const double HUM_MIN                 = 20;
const double HUM_STEP                = 10;
const double STAGE0_RANGE            = 0;
const double STAGE1_ON_RANGE         = 0.9;
const double STAGE2_ON_RANGE         = 2.9; // DO NOT USED
const double STAGE3_ON_RANGE         = 5.9;
const double STAGE1_OFF_RANGE        = 1.0;
const double STAGE2_OFF_RANGE        = 0.9; // F
const double STAGE3_OFF_RANGE        = 4.9;
const double COOLING_ON_RANGE        = 1.9;
const double HEATING_ON_RANGE        = 2.9;
const double ALERT_TIME              = 120;
const double CHANGE_STAGE_TIME       = 40;
const double CHANGE_STAGE_TIME_WO_OB = 10;
const double S2OFF_TIME              = 2;

const double T1 = 0.9; // F
const double T2 = 1.9; // F
const double T3 = 2.9; // F
const double T4 = 3.9; // F
const double T5 = 4.9; // F

// Status backlight color
const QVariantList coolingColor      = QVariantList{0, 128, 255, STHERM::LedEffect::LED_FADE, "true"};
const QVariantList heatingColor      = QVariantList{255, 68, 0, STHERM::LedEffect::LED_FADE,  "true"};
const QVariantList emergencyColor    = QVariantList{255, 0, 0, STHERM::LedEffect::LED_BLINK,  "true"};
const QVariantList emergencyColorS   = QVariantList{255, 0, 0, STHERM::LedEffect::LED_STABLE, "true"};

Scheme::Scheme(DeviceAPI* deviceAPI, QSharedPointer<SchemeDataProvider> schemeDataProvider, QObject *parent) :
    mRestarting (false),
    BaseScheme (deviceAPI, schemeDataProvider, parent)
{

    mTiming = mDeviceAPI->timing();
    mRelay  = Relay::instance();

    mCurrentSysMode = AppSpecCPP::SystemMode::Auto;
    mSwitchDFHActiveSysTypeTo = AppSpecCPP::SystemType::SysTUnknown;
    mActiveSysTypeHeating = AppSpecCPP::SystemType::SysTUnknown;
    mActiveHeatPumpMode = AppSpecCPP::SMUnknown;

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
        auto sys = mDataProvider->systemSetup();
        bool dualFuelLog = sys ? sys->systemType == AppSpecCPP::SystemType::DualFuelHeating : false;

        SCHEME_LOG << "App Version: " << QCoreApplication::applicationVersion();
        LOG_CHECK_SCHEME(isRunning()) << "Scheme Running with these parameters: -------------------------------" << mTiming->totUptime.elapsed();
        LOG_CHECK_SCHEME(isRunning()) << "Temperature scheme version: " << QString(TEMPERATURE_SCHEME_VERSION);
        LOG_CHECK_SCHEME(isRunning() && sys) << "systemMode: " << mDataProvider->effectiveSystemMode() << "systemType: " << sys->systemType;
        LOG_CHECK_SCHEME(isRunning() && mDataProvider->isPerfTestRunning())<< "Perf test actual system-mode: "<< sys->systemMode;
        LOG_CHECK_SCHEME(isRunning() && sys) << "systemRunDelay: " << sys->systemRunDelay << "isVacation: " << mDataProvider->isVacationEffective();
        LOG_CHECK_SCHEME(isRunning() && mDataProvider->isPerfTestRunning())<< "Perf test actual isVacation: "<<  sys->isVacation;
        LOG_CHECK_SCHEME(isRunning() && sys) << "heatStage: "  << sys->heatStage << "coolStage: " <<sys->coolStage;
        LOG_CHECK_SCHEME(isRunning() && sys) << "auxiliaryHeating: "  << sys->auxiliaryHeating << ", heatPumpOBState: " << (sys->heatPumpOBState == 0 ? "cooling" : "heating");
        LOG_CHECK_SCHEME(isRunning() && sys) << "emergencyMinimumTime: " << sys->emergencyMinimumTime;
        LOG_CHECK_SCHEME(isRunning() && sys) << "useAuxiliaryParallelHeatPump: "  << sys->useAuxiliaryParallelHeatPump << ", driveAux1AndETogether: " << sys->driveAux1AndETogether << ", driveAuxAsEmergency: " << sys->driveAuxAsEmergency;
        LOG_CHECK_SCHEME(isRunning() && sys) << "systemAccessories (wire, typ): " << sys->systemAccessories->property("accessoriesWireType") <<  sys->systemAccessories->property("accessoriesType");
        LOG_CHECK_SCHEME(isRunning() && mRelay) << "getOb_state: "  << mRelay->getOb_state() << "relays: " << mRelay->relays().printStr();
        LOG_CHECK_SCHEME(isRunning() && mDataProvider.data()->schedule()) << "Schedule : " << mDataProvider.data()->schedule()->name;
        LOG_CHECK_SCHEME(isRunning()) << "Timing S1UT: " << mTiming->s1uptime.elapsed() << "S2UT: " << mTiming->s2uptime.elapsed();
        LOG_CHECK_SCHEME(isRunning()) << "S2OT: " << mTiming->s2Offtime.elapsed() << "uptime: " << mTiming->uptime.elapsed();
        LOG_CHECK_SCHEME(isRunning()) << "alert:" << mTiming->alerts << "s3hold:" << mTiming->s3hold << "s2hold:" << mTiming->s2hold;
        LOG_CHECK_SCHEME(isRunning() && mFanHourTimer.isActive()) << "fan on 1 hour stage finishes in " << mFanHourTimer.remainingTime() / 60000 << "minutes";
        LOG_CHECK_SCHEME(isRunning() && mFanWPHTimer.isActive()) << "fan on stage finishes in " << mFanWPHTimer.remainingTime() / 60000 << "minutes";
        LOG_CHECK_SCHEME(isRunning()) << "Current Temperature : " << effectiveCurrentTemperature() << "Effective Set Temperature: " << effectiveTemperature();
        LOG_CHECK_SCHEME(isRunning()) << "Current Humidity : " << mDataProvider->currentHumidity() << "Effective Set Humidity: " << mDataProvider->effectiveSetHumidity();

        LOG_CHECK_SCHEME(isRunning() && dualFuelLog) << "Current Outdoor temperature: " << mDataProvider->outdoorTemperatureF()
                                                     << "dualFuelManualHeating: " << sys->dualFuelManualHeating
                                                     << "dualFuelHeatingModeDefault: " << sys->dualFuelHeatingModeDefault;

        LOG_CHECK_SCHEME(isRunning() && dualFuelLog) << "isAUXAuto: " << sys->isAUXAuto
                                                     << "runFanWithAuxiliary: " << sys->runFanWithAuxiliary
                                                     << "dualFuelThreshold: " << sys->dualFuelThreshold;

        LOG_CHECK_SCHEME(!isRunning()) << "-----------------------------Scheme is stopped ---------------------";
        LOG_CHECK_SCHEME(isRunning())  << "-----------------------------Scheme Log Ended  ---------------------";
    });

    connect(mDataProvider.get(), &SchemeDataProvider::outdoorTemperatureReady, this, [this] () {
        SCHEME_LOG << "outdoorTemperatureReady" << mActiveSysTypeHeating << mDataProvider->systemSetup()->systemMode << mSwitchDFHActiveSysTypeTo;
        LOG_CHECK_SCHEME(mDataProvider->isPerfTestRunning())<< "outdoorTemperatureReady" << mDataProvider->effectiveSystemMode();

        // Device has internet and outdoor temperature has been successfully updated, so move to automatic algorithm.
        switchDFHActiveSysType(AppSpecCPP::SystemType::SysTUnknown);
    });

    connect(mDataProvider.get(), &SchemeDataProvider::outdoorTemperatureChanged, this, [this] () {
        auto system = mDataProvider->systemSetup();
        SCHEME_LOG << "outdoorTemperatureChanged" << mActiveSysTypeHeating << system->systemMode;
        LOG_CHECK_SCHEME(mDataProvider->isPerfTestRunning())<< "outdoorTemperatureChanged" << mDataProvider->effectiveSystemMode();
        if (system->systemType == AppSpecCPP::SystemType::DualFuelHeating && system->isAUXAuto) {
            checkForRestart();
        }
    });

    connect(mDataProvider.get(), &SchemeDataProvider::currentTemperatureChanged, this, [this] () {
        SCHEME_LOG << "currentTemperatureChanged";
        checkForRestart();
    });

    connect(mDataProvider.get(), &SchemeDataProvider::setTemperatureChanged, this, [this] () {
        SCHEME_LOG << "setTemperatureChanged";
        checkForRestart();
    });
}

void Scheme::stop()
{
    SCHEME_LOG << "stopping HVAC" ;

    if (mLogTimer.isActive())
        mLogTimer.stop();

    if (mUpdatingTimer.isActive())
        mUpdatingTimer.stop();

    if (mFanHourTimer.isActive())
        mFanHourTimer.stop();

    if (mFanWPHTimer.isActive())
        mFanWPHTimer.stop();

    stopWork = true;
    emit stopWorkRequested();

    // Stop worker.
    terminate();
    SCHEME_LOG << "terminated HVAC" ;
    wait();

    SCHEME_LOG << "stopped HVAC" ;
}

Scheme::~Scheme()
{
    stop();
}

void Scheme::restartWork(bool forceStart)
{
    if (isRunning()) {
        SCHEME_LOG << "restarting HVAC" << stopWork;
        if (stopWork) // restart is already in progress
            return;
        // Any finished signal should not start the worker.
        connect(
            this,
            &Scheme::finished,
            this,
            [=]() {
                SCHEME_LOG << "restarted HVAC";
                stopWork = false;
                mLogTimer.start();

                // It will be called when heating loop started in dual fuel mode.
                emit dfhSystemTypeChanged(AppSpecCPP::SysTUnknown);

                this->start();
            },
            Qt::SingleShotConnection);

        stopWork = true;
        emit stopWorkRequested();
        this->wait(QDeadlineTimer(1000, Qt::PreciseTimer));

    } else if (forceStart){
        SCHEME_LOG << "started HVAC";
        stopWork = false;
        mLogTimer.start();

        // It will be called when heating loop started in dual fuel mode.
        emit dfhSystemTypeChanged(AppSpecCPP::SysTUnknown);

        this->start();
    } else {
        SCHEME_LOG << "trying to start before main start";
    }
}

void Scheme::run()
{
    SCHEME_LOG << "-- startWork is running." << QThread::currentThreadId();

    mTiming->totUptime.start();

    if (!mDataProvider.data()->systemSetup()) {
        SCHEME_LOG << "-- SystemSetup is not ready.";
        return;
    }

    //! get wires config type modes ...
    updateParameters();

    //! reset delays
    resetDelays();

    while (!stopWork) {

        // Vacation has a higher priority compared to other processes.
        if (mDataProvider.data()->isVacationEffective()) {
            VacationLoop();

        } else {
            switch (mDataProvider.data()->effectiveSystemMode()) {
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
            case AppSpecCPP::SystemMode::EmergencyHeat:
                manualEmergencyHeating();
                break;

            case AppSpecCPP::SystemMode::Emergency:
                EmergencyLoop();
                break;
            default:
                qWarning() << "Unsupported Mode in controller loop" << mDataProvider->effectiveSystemMode() << "system mode:" << mDataProvider.data()->systemSetup()->systemMode;
                break;
            }
        }

        // reset to avoid excess restart while values are changing
        mActiveSysTypeHeating = AppSpecCPP::SystemType::SysTUnknown;

        if (!mRestarting) {
            mRelay->setAllOff();

            //        int fanWork = QDateTime::currentSecsSinceEpoch() - mTiming->fan_time.toSecsSinceEpoch() - mFanWPH - 1;
            //        mRelay->fanWorkTime(mFanWPH, fanWork);

            sendRelays(true);
        }

        if (stopWork)
            break;

        // all should be off! we can assert here
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
        // wait for change or at least wait 10 seconds preventing too much calling
        waitLoop();
    }

    SCHEME_LOG << "-- startWork Stopped. working time(ms): " << mTiming->totUptime.elapsed();
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
    auto effectiveTemp = effectiveTemperature();
    auto effectiveCurrentTemp = effectiveCurrentTemperature();

    if (effectiveCurrentTemp > effectiveTemp) {
        CoolingLoop();
    } else if (effectiveCurrentTemp < effectiveTemp) {
        HeatingLoop();
    }
}

void Scheme::CoolingLoop()
{
    LOG_CHECK_SCHEME(false) << "Cooling started " << mDataProvider.data()->systemSetup()->systemType;

    bool heatPump = false;
    // s1 time threshold
    // Y1, Y2, O/B G config

    switch (mDataProvider.data()->systemSetup()->systemType) { // Device type
    case AppSpecCPP::SystemType::HeatPump:
    case AppSpecCPP::SystemType::DualFuelHeating: // Works same as HeatPump in cooling
        //! InternalCoolingLoopStage1 uses the cooling stage as a heat pump (both determine the Y wires),
        //!  and they're identical without causing any issues.
        heatPump = true;
    case AppSpecCPP::SystemType::Conventional:
    case AppSpecCPP::SystemType::CoolingOnly: {
        LOG_CHECK_SCHEME(false) << heatPump << effectiveCurrentTemperature() << effectiveTemperature();

        bool hasDelay = false;
        while (!stopWork && effectiveCurrentTemperature() - effectiveTemperature() >= T1) {

            //    if (pumpHeat) // how the system type setup get OB Orientatin, kept for later
            // always set but not send relays update if not heat pump internally
            bool obUpdated = updateOBState(AppSpecCPP::Cooling);
            if (heatPump && obUpdated && !stopWork){
                // sysDelay
                runSystemDelay(AppSpecCPP::Cooling);

            } else if (mTiming->s1Offtime.isValid() && mTiming->s1Offtime.elapsed() < 2 * 60 * 1000) {
                if (!hasDelay) {
                    hasDelay = true;

                    // This delay is not a system delay, anyway we send it to UI.
                    emit startSystemDelayCountdown(AppSpecCPP::Cooling, 2 * 60 * 1000 - mTiming->s1Offtime.elapsed());
                }
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);

                if (!stopWork)
                    continue;
            }

            // Stop countdown, delay is finished.
            // We should not have any return or break before this unless we handle the stop
            if (hasDelay) {
                emit stopSystemDelayCountdown();
            }

            // check again after wait
            if (stopWork)
                return;

            emit actualModeStarted(AppSpecCPP::Cooling);

            internalCoolingLoopStage1();
            break;
        }
    } break;
    default:
        qWarning() << "Unsupported system type in Cooling loop" << mDataProvider.data()->systemSetup()->systemType;
        break;
    }

    // turn off Y1, Y2 and G = 0 outside
}

AppSpecCPP::SystemType Scheme::activeSystemTypeHeating() {
    // Control the system type:
    auto activeSysType = mDataProvider->systemSetup()->systemType;
    if (activeSysType == AppSpecCPP::SystemType::DualFuelHeating) {
        if (mSwitchDFHActiveSysTypeTo != AppSpecCPP::SystemType::SysTUnknown) {
            activeSysType = mSwitchDFHActiveSysTypeTo;

        } else if (!mDataProvider->systemSetup()->isAUXAuto) {
            // Select active system type based on the selected heating type in the system mode page.
            auto dualFuelManualHeating = mDataProvider->systemSetup()->dualFuelManualHeating;

            // Use default dual fuel heating mode in Auto or Vacation.
            if (mDataProvider->effectiveSystemMode() == AppSpecCPP::Auto ||
                mDataProvider->isVacationEffective()) {
                dualFuelManualHeating = mDataProvider->systemSetup()->dualFuelHeatingModeDefault;
            }

            if (dualFuelManualHeating == AppSpecCPP::DFMAuxiliary) {
                activeSysType = AppSpecCPP::SystemType::HeatingOnly;

            } else {
                // Default
                activeSysType = AppSpecCPP::SystemType::HeatPump;

                LOG_CHECK_SCHEME(dualFuelManualHeating == AppSpecCPP::DFMOff)
                    << mDataProvider->effectiveSystemMode()
                    << mDataProvider->isVacationEffective()
                    << mDataProvider->systemSetup()->dualFuelHeatingModeDefault;
            }

            // in the cold outdoor heatpump can not function good so we use it only above threshold
        } else if (mDataProvider->outdoorTemperatureF() > mDataProvider->dualFuelThresholdF()) {
            // Start the heat pump (Y wires)
            activeSysType = AppSpecCPP::SystemType::HeatPump;
        } else {
            // Start the heating using Aux (W wires)
            activeSysType = AppSpecCPP::SystemType::HeatingOnly;
        }
    }

    return activeSysType;
}

void Scheme::switchDFHActiveSysType(AppSpecCPP::SystemType to)
{
    if (to != mSwitchDFHActiveSysTypeTo) {
        mSwitchDFHActiveSysTypeTo = to;

        if (mDataProvider->systemSetup()->systemType == AppSpecCPP::SystemType::DualFuelHeating) {
            checkForRestart();
        }
    }
}

void Scheme::updateHeatPumpProperties() {
    // In heat pump mode we use cool stage as heat pump stage that control the Y wires
    if (mDataProvider->systemSetup()->coolStage == 2) {
        _HPT1  = T1;
        _HPT2  = T2;
        _AUXT1 = T3;
        _AUXT2 = T4;

    } else {
        // Default: we consider the cool stage is 1
        _HPT1  = T1;
        _AUXT1 = T2;
        _AUXT2 = T3;
    }
}

void Scheme::HeatingLoop()
{
    LOG_CHECK_SCHEME(false) << "Heating started " << mDataProvider.data()->systemSetup()->systemType;

    mActiveSysTypeHeating = activeSystemTypeHeating();
    emit dfhSystemTypeChanged(mActiveSysTypeHeating);

    // update configs and ...
    // s1 & s2 time threshold
    //    Y1, Y2, O/B G W1 W2 W3
    switch (mActiveSysTypeHeating) {
    case AppSpecCPP::SystemType::HeatPump: // emergency as well?
    {
        LOG_CHECK_SCHEME(false) << "HeatPump" << mDataProvider.data()->currentTemperature() << effectiveTemperature();
        SCHEME_LOG << "Heat pump, heating, " << mDataProvider->systemSetup()->systemType << mActiveSysTypeHeating;

        // Heat pump with auxiliary when the actual system type is heat pump
        if (mDataProvider->systemSetup()->systemType == AppSpecCPP::HeatPump &&
            mDataProvider->systemSetup()->auxiliaryHeating) {
            SCHEME_LOG << "Heat pump with Aux " << mDataProvider->systemSetup()->auxiliaryHeating;

            emit actualModeStarted(AppSpecCPP::Heating);
            updateHeatPumpProperties();
            internalPumpHeatingWithAuxLoopStage1();

            // When the active system type is Heat pump, the actual system type can be dual fuel
        } else if (mDataProvider.data()->currentTemperature() < effectiveTemperature()) {
            emit actualModeStarted(AppSpecCPP::Heating);

            SCHEME_LOG << "Normal heat pump";
            internalPumpHeatingLoopStage1();
        }
    } break;
    case AppSpecCPP::SystemType::Conventional:
    case AppSpecCPP::SystemType::HeatingOnly:
        SCHEME_LOG << "Conventional" << effectiveCurrentTemperature() << effectiveTemperature();
        if (effectiveTemperature() - effectiveCurrentTemperature() >= T1) {
            emit actualModeStarted(AppSpecCPP::Heating);
            internalHeatingLoopStage1();
        }
        break;
    default:
        qWarning() << "Unsupported system type in Heating loop" << mDataProvider.data()->systemSetup()->systemType;
        break;
    }

    // Turn off system (Y1, Y2, W1, W2,W3, G = 0) outside
}

void Scheme::VacationLoop()
{
    if ((mVacationMinimumTemperature - mDataProvider.data()->currentTemperature()) > 0.001) {
        HeatingLoop();

    } else if ((mVacationMaximumTemperature - mDataProvider.data()->currentTemperature()) < 0.001) {
        CoolingLoop();

    } else {
        SCHEME_LOG << "The conditions for the vacation loop (Temperature scheme) are not met.";
        waitLoop(-1);
    }
}

void Scheme::EmergencyLoop()
{
    //TODO
    waitLoop();
}

void Scheme::OffLoop()
{
    // we check for system setup and if that is in default state, we should set relays off as well
    // so we skip the waitloop to go for relays
    if (mDataProvider->systemSetup()->systemType == AppSpecCPP::SystemType::SysTUnknown)
        return;

    waitLoop(-1, AppSpecCPP::ctMode);
}

bool Scheme::updateOBState(AppSpecCPP::SystemMode newOb_state)
{
    auto sysType = mDataProvider.data()->systemSetup()->systemType;
    // we should check if it is changed or not!
    if (mRelay->setOb_state(newOb_state) && (sysType == AppSpecCPP::HeatPump || sysType == AppSpecCPP::DualFuelHeating))
    {
        sendRelays();
        return  true;
    }
    return false;
}

void Scheme::internalCoolingLoopStage1()
{
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
    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

    while (!stopWork && effectiveTemperature() - effectiveCurrentTemperature() < STAGE1_OFF_RANGE) {
        SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mRelay->relays().y2
                   << mDataProvider.data()->systemSetup()->coolStage << mTiming->s1uptime.elapsed()
                   << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();

        // coolStage: In the heat pump or dual fuel heating the cool stage and heat pump stage are the same.
        if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mDataProvider->systemSetup()->coolStage == 2) {
            if (effectiveCurrentTemperature() - effectiveTemperature() >= T2 ||
                (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 10 * 60000)) {
                if (!mTiming->s2Offtime.isValid() || mTiming->s2Offtime.elapsed() >= 2 * 60000) {
                    // going to stage 2 and if return false break to go off
                    if (!internalCoolingLoopStage2()) {
                        break;
                    }
                }

                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
                // avoid sending alert either it went to stage2 and returned back or off time not yet elapsed
                continue;
            }
        }

        sendAlertIfNeeded();
        if (stopWork)
            break;
        waitLoop(30000);

        if (stopWork)
            break;

        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
    }

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << "finished cooling, stop work: " << stopWork;

    mTiming->s1Offtime.restart();
}

bool Scheme::internalCoolingLoopStage2()
{
    if (stopWork)
        return false;

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold;

    // turn on stage 2
    mRelay->coolingStage2();
    // 5 Sec
    emit changeBacklight(coolingColor);

    sendRelays();

    while (!stopWork) {
        SCHEME_LOG << mTiming->s2hold << effectiveTemperature() << effectiveCurrentTemperature();
        if (mTiming->s2hold) {
            if (effectiveTemperature() - effectiveCurrentTemperature() < STAGE1_OFF_RANGE) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (effectiveCurrentTemperature() - effectiveTemperature() > T1) {
                sendAlertIfNeeded();
            } else {
                break;
            }
        }

        if (stopWork)
            break;

        waitLoop(30000);
    }

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << "finished stage 2" << stopWork;

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
    if (!stopWork){
        // sysDelay
        runSystemDelay(AppSpecCPP::Heating);
    }

    if (stopWork)
        return;

    // 5 secs
    emit changeBacklight(heatingColor);

    mTiming->s1uptime.restart();
    mTiming->uptime.restart();
    mTiming->s2hold = false;
    mTiming->s3hold = false;
    mTiming->alerts = false;

    mRelay->heatingStage1();
    sendRelays();
    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

    while (!stopWork && effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
        LOG_CHECK_SCHEME(true) << mRelay->relays().w2 << mDataProvider->systemSetup()->heatStage
                               << mTiming->s1uptime.isValid() << mTiming->s1uptime.elapsed();
        SCHEME_LOG << "Current Temperature: " << effectiveCurrentTemperature() << ", Effective Temperature: " << effectiveTemperature();

        if (mRelay->relays().w2 != STHERM::RelayMode::NoWire && mDataProvider->systemSetup()->heatStage >= 2) {
            if (effectiveTemperature() - effectiveCurrentTemperature() >= T2
                || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 10 * 60000)) {
                // enabling stage 2 and if returned false we break to go off
                if (!internalHeatingLoopStage2())
                    break;

                // avoid sending alert either it went to stage2 and returned back
                continue;
            }
        }

        sendAlertIfNeeded();
        if (stopWork)
            break;
        waitLoop(30000);

        if (stopWork)
            break;

        // TODO should we wait for temp update before new loop?
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
    }

    SCHEME_LOG << mDataProvider.data()->currentTemperature() << effectiveTemperature() << "finished heating conventional" << stopWork;

    mTiming->s1Offtime.restart();
    // will turn off all outside
}

bool Scheme::internalHeatingLoopStage2()
{
    if (stopWork)
        return false;

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold;

    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s2uptime.restart();

    mRelay->heatingStage2();
    sendRelays();
    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

    while (!stopWork) {
        SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold;

        if (mTiming->s2hold) {
            if (effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
                if ((mRelay->relays().w3 == STHERM::RelayMode::NoWire || mDataProvider->systemSetup()->heatStage < 3)
                    || (mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() < 10 * 60000)) { // this is valid for sure, checking for sanity
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
            bool timeCriteria = !mTiming->s2uptime.isValid() || mTiming->s2uptime.elapsed() >= 10 * 60000;
            if ((effectiveTemperature() - effectiveCurrentTemperature() < T2 && !timeCriteria) ||
                (mRelay->relays().w3 == STHERM::RelayMode::NoWire || mDataProvider.data()->systemSetup()->heatStage < 3)) {
                if (effectiveTemperature() - effectiveCurrentTemperature() < T1) {
                    break;
                } else {
                    sendAlertIfNeeded();
                }
                if (stopWork)
                    break;
                waitLoop(30000);
            } else {
                if (effectiveTemperature() - effectiveCurrentTemperature() < T3
                    && !timeCriteria) {
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

        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
    }

    SCHEME_LOG << mDataProvider.data()->currentTemperature() << effectiveTemperature() << "finished stage 2 heat" << stopWork;

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
    if (stopWork)
        return false;

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s3hold;

    mRelay->heatingStage3();
    // 5 secs
    emit changeBacklight(heatingColor);
    mTiming->s2hold = false;
    sendRelays();

    while (!stopWork) {
        SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s3hold;
        if (mTiming->s3hold) {
            if (effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (effectiveTemperature() - effectiveCurrentTemperature() < T3) {
                break;
            } else {
                sendAlertIfNeeded();
            }
        }
        if (stopWork)
            break;

        waitLoop(30000);
    }
    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << "finished stage 3" << stopWork;

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
    if (stopWork)
        return;

    mActiveHeatPumpMode =  AppSpecCPP::Heating;

    bool hasDelay = false;
    while (effectiveTemperature() - mDataProvider.data()->currentTemperature() >= STAGE1_ON_RANGE) {
        auto obUpdated = updateOBState(AppSpecCPP::Heating);

        if (obUpdated && !stopWork){
            // sysDelay
            runSystemDelay(AppSpecCPP::Heating);

        } else if (mTiming->s1Offtime.isValid() &&
                   mTiming->s1Offtime.elapsed() < 2 * 60 * 1000){
            if (!hasDelay) {
                hasDelay = true;
                emit startSystemDelayCountdown(AppSpecCPP::Heating, 2 * 60 * 1000 - mTiming->s1Offtime.elapsed());
            }

            waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);

            if (!stopWork)
                continue;
        }

        // Stop countdown
        if (hasDelay) {
            emit stopSystemDelayCountdown();
            hasDelay = false;
        }

        // check again after wait
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
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

        while (!stopWork && effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
            SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mRelay->relays().y2
                       << mDataProvider->heatPumpStage() << mTiming->s1uptime.elapsed()
                       << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();

            if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mDataProvider->heatPumpStage() >= 2) {
                if (effectiveTemperature() - effectiveCurrentTemperature() >= T2
                    || (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 10 * 60000)) {
                    if (!mTiming->s2Offtime.isValid() || mTiming->s2Offtime.elapsed() >= 2 * 60000) {
                        // go to stage 2 and if return false break to go off
                        if (!internalPumpHeatingLoopStage2()) {
                            break;
                        }
                    }

                    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
                    // avoid sending alert either it went to stage2 and returned back or off time not yet elapsed
                    continue;
                }
            }

            sendAlertIfNeeded();
            // wait for temperature update?
            if (stopWork)
                break;
            waitLoop(30000);

            if (stopWork)
                break;

            waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
        }

        mTiming->s1Offtime.restart();
        break;
    }

    // Stop countdown
    if (hasDelay) {
        emit stopSystemDelayCountdown();
    }

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << "finished pump heat" << stopWork;
    mActiveHeatPumpMode = AppSpecCPP:: SMUnknown;
}

bool Scheme::internalPumpHeatingLoopStage2()
{
    if (stopWork)
        return false;

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold;
    // turn on stage 2
    mRelay->heatingStage2(true);
    // 5 Sec
    emit changeBacklight(heatingColor);
    sendRelays();

    while (!stopWork) {
        SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold;

        if (mTiming->s2hold) {
            if (effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
                sendAlertIfNeeded();
            } else {
                return false;
            }
        } else {
            if (effectiveTemperature() - effectiveCurrentTemperature() > STAGE1_ON_RANGE) {
                sendAlertIfNeeded();
            } else {
                break;
            }
        }

        if (stopWork)
            break;

        waitLoop(30000);
    }
    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << "finished stage 2 pump" << stopWork;

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

void Scheme::internalPumpHeatingWithAuxLoopStage1()
{
    if (stopWork)
        return;

    mActiveHeatPumpMode =  AppSpecCPP::Heating;

    bool hasDelay = false;
    while (effectiveTemperature() - effectiveCurrentTemperature() >= T1) {
        bool obUpdated = false;

        if (mRelay->getOb_state() != AppSpecCPP::Heating) {
            obUpdated = updateOBState(AppSpecCPP::Heating);
        }

        if (obUpdated && !stopWork){
            // sysDelay because ob state is not "heating".
            runSystemDelay(AppSpecCPP::Heating);

        } else if (mTiming->s1Offtime.isValid() &&
                   mTiming->s1Offtime.elapsed() < 2 * 60 * 1000) {
            // This is not a system delay, this is due to s1Offtime.
            if (!hasDelay) {
                hasDelay = true;
                emit startSystemDelayCountdown(AppSpecCPP::Heating, 2 * 60 * 1000 - mTiming->s1Offtime.elapsed());
            }

            waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);

            if (!stopWork)
                continue;
        }

        // Stop countdown
        if (hasDelay) {
            emit stopSystemDelayCountdown();
            hasDelay = false;
        }

        // check again after wait
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
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

        while (!stopWork && effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
            SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << mRelay->relays().y2
                       << mDataProvider->heatPumpStage() << mTiming->s1uptime.elapsed()
                       << mTiming->s2Offtime.isValid() << mTiming->s2Offtime.elapsed();

            if (effectiveTemperature() - effectiveCurrentTemperature() < T2 &&
                (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() < 10 * 60000)) {
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
                continue;
            }

            if (mRelay->relays().y2 != STHERM::RelayMode::NoWire && mDataProvider->heatPumpStage() >= 2) {
                if (!mTiming->s2Offtime.isValid() || mTiming->s2Offtime.elapsed() >= 2 * 60000) {
                    auto HPStage2 = internalPumpHeatingWithAuxLoopStage2();
                    if (HPStage2 == Break) {
                        break;

                    } else if (HPStage2 == Continue) {
                        // no need to wait some moments because it will be done in internalPumpHeatingWithAuxLoopStage2
                        continue;
                    }

                } else {
                    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
                    continue;
                }

            } else {
                if (effectiveTemperature() - effectiveCurrentTemperature() >= _AUXT1 &&
                    auxiliaryHeatingLoopStage1(true)) {
                    break;

                } else {
                    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
                    continue;
                }
            }

            if (stopWork)
                break;

            waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
        }

        mTiming->s1Offtime.restart();
        break;
    }

    // Stop countdown
    if (hasDelay) {
        emit stopSystemDelayCountdown();
    }

    SCHEME_LOG << effectiveCurrentTemperature() << effectiveTemperature() << "finished pump heat" << stopWork;
    mActiveHeatPumpMode = AppSpecCPP:: SMUnknown;
}

Scheme::ReturnType Scheme::internalPumpHeatingWithAuxLoopStage2()
{
    if (stopWork)
        return Break;

    SCHEME_LOG << "Start the internalPumpHeatingWithAuxLoopStage2 loop" << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold << _HPT1;

    // turn on stage 2
    mRelay->heatingStage2(true);

    mTiming->s2uptime.restart();

    // 5 Sec
    emit changeBacklight(heatingColor);
    sendRelays();

    while (!stopWork) {

        if (effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
            if (effectiveTemperature() - effectiveCurrentTemperature() <= _HPT1) {
                break;

            } else if (effectiveTemperature() - effectiveCurrentTemperature() >= _AUXT1 || // _AUXT1 is T3 for sure
                mTiming->s2uptime.isValid() && mTiming->s2uptime.elapsed() >= 10 * 60000) {
                // Go to the AUX
                if (auxiliaryHeatingLoopStage1(true)) {
                    SCHEME_LOG << "End the internalPumpHeatingWithAuxLoopStage2 loop after auxiliaryHeatingLoopStage1";
                    return Break;
                } else {
                    Q_ASSERT_X(true, "auxiliaryHeatingLoopStage1", " continue case is not valid.");
                    //! //TODO we need to handle all correctly here such as updating relay parameters etc..
                    return Continue;
                }

            } else {
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctMode);
                continue;
            }

        } else {
            SCHEME_LOG << "End the internalPumpHeatingWithAuxLoopStage2 loop";
            return Break;
        }

        if (stopWork)
            break;

        waitLoop(30000);
    }

    SCHEME_LOG << "End the internalPumpHeatingWithAuxLoopStage2 loop" << effectiveCurrentTemperature() << effectiveTemperature() << mTiming->s2hold << _HPT1;

    if (stopWork)
        return Break;

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

    return Successful;
}

bool Scheme::auxiliaryHeatingLoopStage1(bool afterHP)
{
    SCHEME_LOG << "Start: auxiliaryHeatingLoopStage1 " << stopWork << mDataProvider->systemSetup()->driveAux1AndETogether << afterHP;

    if (stopWork)
        return true;


    if (mDataProvider->systemSetup()->auxiliaryHeating) {
        if (!afterHP && effectiveTemperature() - effectiveCurrentTemperature() < _AUXT1) {
            return true;
        }

        // 5 secs
        emit changeBacklight(heatingColor);
        emit auxiliaryStatusChanged(true);

        mTiming->s1uptime.restart();
        mTiming->s2Offtime.invalidate(); // Check: s2 off time should be high value? or invalid?
        mTiming->uptime.restart();
        mTiming->s2hold = false;
        mTiming->alerts = false;

        // Update relays
        if (!mDataProvider->systemSetup()->useAuxiliaryParallelHeatPump) {
            mRelay->turnOffHeatPump();
        }

        mRelay->auxiliaryHeatingStage1(mDataProvider->systemSetup()->driveAux1AndETogether);
        sendRelays();
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

        while (!stopWork && effectiveCurrentTemperature() - effectiveTemperature() < STAGE1_OFF_RANGE) {
            if (mDataProvider->systemSetup()->heatStage >= 2 &&
                (effectiveTemperature() - effectiveCurrentTemperature() >= _AUXT2 ||
                 (mTiming->s1uptime.isValid() && mTiming->s1uptime.elapsed() >= 10 * 60000))) {

                if (auxiliaryHeatingLoopStage2())
                    break;

            } else {
                sendAlertIfNeeded();

                if (stopWork)
                    break;

                waitLoop(30000);
            }
        }
    }

    emit auxiliaryStatusChanged(false);
    SCHEME_LOG << "End: auxiliaryHeatingLoopStage1: " << stopWork << effectiveTemperature() << effectiveCurrentTemperature() << _AUXT2;

    return true;
}

bool Scheme::auxiliaryHeatingLoopStage2()
{
    if (stopWork)
        return true;

    SCHEME_LOG << "Start: auxiliaryHeatingLoopStage2: " << mTiming->s2hold;

    // 5 secs
    emit changeBacklight(heatingColor);
    mRelay->auxiliaryHeatingStage2();
    sendRelays();
    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);


    while (!stopWork) {
        if (mTiming->s2hold) {
            if (effectiveCurrentTemperature() - effectiveTemperature() >= STAGE1_OFF_RANGE) {
                SCHEME_LOG << "End: auxiliaryHeatingLoopStage2: " << mTiming->s2hold;
                return true;
            }

        } else if (effectiveTemperature()- effectiveCurrentTemperature() <= _AUXT1) {
            break;
        }

        sendAlertIfNeeded();
        waitLoop(30000);
    }

    if (!mRestarting && !stopWork) {
        mRelay->auxiliaryHeatingStage2(false);
        mRelay->auxiliaryHeatingStage1(mDataProvider->systemSetup()->driveAux1AndETogether);

        // 5 secs
        emit changeBacklight(heatingColor);
        mTiming->s1uptime.restart();
        mTiming->s2hold = true;
        mTiming->s2Offtime.restart();
        sendRelays();
    }

    SCHEME_LOG << "End: auxiliaryHeatingLoopStage2" << stopWork;

    return false;
}

void Scheme::manualEmergencyHeating()
{
    SCHEME_LOG << "Start the emergency heating mode ";

    // Optimize emergency mode to prevent unnecessary backlight and emergency heating activation.
    // This will restrict the activation of emergency heating to instances
    // where there's a significant change in temperature to met the temperature conditions.
    if (effectiveTemperature() - effectiveCurrentTemperature() <= 0) {
        SCHEME_LOG << "System should be off, the conditions are not changed!";
        return;
    }

    emergencyHeatingLoop();
}

void Scheme::emergencyHeatingLoop()
{
    auto sysSetup = mDataProvider->systemSetup();

    // Sanity check
    if (sysSetup->systemType != AppSpecCPP::HeatPump) {
        SCHEME_LOG << "The system type is incompatible with the manual emergency mode, The emergency mode can be activate only when system type is heat pump . "
                   << sysSetup->systemType;
        return;
    }

    if (!sysSetup->auxiliaryHeating) {
        SCHEME_LOG << "Auxiliary heating is off.";

        return;
    }

    mActiveHeatPumpMode =  AppSpecCPP::EmergencyHeat;
    mRelay->setAllOff();
    mRelay->emergencyAuxiliaryHeating(sysSetup->heatStage == 1 || !sysSetup->driveAuxAsEmergency,
                                      sysSetup->driveAux1AndETogether);

    mTEONTimer.restart();

    // Block the UI
    emit manualEmergencyModeUnblockedAfter(sysSetup->emergencyMinimumTime * 60 * 1000);

    // Emergency wirings will be active due to the reset of the mTEONTimer.
    // 5 Sec
    emit changeBacklight(emergencyColor, emergencyColorS);

    SCHEME_LOG << "Emergency heating - " << "effectiveTemperature: " << effectiveTemperature()
               << " - currentTemperature: " << effectiveCurrentTemperature();


    while (!stopWork && (effectiveTemperature() - effectiveCurrentTemperature() > -1 ||
                         (mTEONTimer.isValid() && sysSetup->emergencyMinimumTime * 60 * 1000 > mTEONTimer.elapsed()))) {


        // Disable UI interactions in system mode page during manual or auto emergency mode until the minimum duration is reached.
        if (mTEONTimer.isValid()) {
            auto remainigEmergencyMinimumTimeMS = sysSetup->emergencyMinimumTime * 60 * 1000 - mTEONTimer.elapsed();
            if (remainigEmergencyMinimumTimeMS > 0) {
                emit manualEmergencyModeUnblockedAfter(remainigEmergencyMinimumTimeMS);

            } else {
                emit manualEmergencyModeUnblockedAfter(0);
                mTEONTimer.invalidate();
            }
        }

        if (effectiveTemperature() - effectiveCurrentTemperature() > -1) {
            sendAlertIfNeeded(ATEmergency);
        }

        // we need break condition here!
        if (sysSetup->systemType != AppSpecCPP::HeatPump || stopWork) {
            break;
        }

        sendRelays();
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
    }

    // To unblock system mode UI in emergency states like system mode or system type changes.
    // Unblock immediately
    emit manualEmergencyModeUnblockedAfter(0);
    mTEONTimer.invalidate();

    emit changeBacklight();
    mRelay->turnOffEmergencyHeating();
    sendRelays();
    waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);

    mActiveHeatPumpMode = AppSpecCPP:: SMUnknown;
}

void Scheme::sendAlertIfNeeded(AlertType alertType)
{
    if (mTiming->alerts)
        return;

    switch (alertType) {
    case ATEmergency:
        mTiming->alerts = mTEONTimer.isValid() && mTEONTimer.elapsed() >= 30 * 60000;
        break;
    case ATNone:
        mTiming->alerts = mTiming->uptime.isValid() && mTiming->uptime.elapsed() >= 120 * 60000;
        break;
    default:
        break;
    }

    if (mTiming->alerts) {
        SCHEME_LOG << "Alert type: " << alertType;
        emit alert();
    }
}

void Scheme::sendRelays(bool forceSend)
{
    if (!forceSend && stopWork)
        return;

    auto relaysConfig = mRelay->relays();
    auto lastConfigs = mRelay->relaysLast();

    // To prevent initial mismatches in device relays after updates, we'll skip the comparison on the first run.
    if (mDataProvider->isRelaysInitialized() &&
        (lastConfigs == relaysConfig)) {
        LOG_CHECK_SCHEME(false) << "no change";
        return;
    }

    if (!mCanSendRelay) {
        int exitCode = waitLoop(-1, AppSpecCPP::ctSendRelay);
        if (exitCode != AppSpecCPP::ctSendRelay){
            mRelay->setRelaysLast(mRelay->relays());
            restartWork();
            return;
        }
        if (!forceSend && stopWork)
            return;
    }

    // To ensure the humidity relays updated.
    mRelay->setRelaysLast(relaysConfig);

    setIsSendingRelays(true);

    if (!mDataProvider->isRelaysInitialized()) {
        // Send the last relays
        emit updateRelays(relaysConfig, true);
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
    }

    mDataProvider->setIsRelaysInitialized(true);

    if (debugMode) {
        auto steps = lastConfigs.changeStepsSorted(relaysConfig);
        for (int var = 0; var < steps.size(); ++var) {
            //! stop sending if not force (for quiting quickly when should align with backdoor)
            if (!forceSend && !mCanSendRelay)
                break;
            auto step = steps.at(var);
            SCHEME_LOG << step.first.c_str() << step.second;
            if (step.first == "o/b"){
                lastConfigs.o_b = relaysConfig.o_b;
                SCHEME_LOG << relaysConfig.o_b;
            }
            else if (step.first == "g"){
                lastConfigs.g = relaysConfig.g;
                SCHEME_LOG << relaysConfig.g;
            }
            else if (step.first == "y1"){
                lastConfigs.y1 = relaysConfig.y1;
                SCHEME_LOG << relaysConfig.y1;
            }
            else if (step.first == "y2"){
                lastConfigs.y2 = relaysConfig.y2;
                SCHEME_LOG << relaysConfig.y2;
            }
            else if (step.first == "w1"){
                lastConfigs.w1 = relaysConfig.w1;
                SCHEME_LOG << relaysConfig.w1;
            }
            else if (step.first == "w2"){
                lastConfigs.w2 = relaysConfig.w2;
                SCHEME_LOG << relaysConfig.w2;
            }
            else if (step.first == "w3"){
                lastConfigs.w3 = relaysConfig.w3;
                SCHEME_LOG << relaysConfig.w3;
            } else {
                // To ignore humidity relays
                continue;
            }

            // Update relays
            if (step.second != 0) {
                emit updateRelays(lastConfigs);
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }
    } else { // Update relays
        emit updateRelays(relaysConfig);
    }

    emit currentSystemModeChanged(mRelay->currentState(),
                                  mRelay->currentHeatingStage(),
                                  mRelay->currentCoolingStage());

    LOG_CHECK_SCHEME(false) << "finished";

    setIsSendingRelays(false);
}

int Scheme::waitLoop(int timeout, AppSpecCPP::ChangeTypes overrideModes)
{
    QEventLoop loop;
    // connect signal for handling stopWork
    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctCurrentTemperature)){
        connect(this, &Scheme::currentTemperatureChanged, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctCurrentTemperature);
        });
    }

    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctSetTemperature)){
        connect(this, &Scheme::setTemperatureChanged, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctSetTemperature);
        });
    }

    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctMode)){
        connect(this, &Scheme::stopWorkRequested, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctMode);
        });
    }

    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctSendRelay)){
        connect(this, &Scheme::canSendRelay, &loop, [&loop](bool restart) {
            loop.exit(restart ? AppSpecCPP::ChangeType::ctNone : AppSpecCPP::ChangeType::ctSendRelay);
        });

        //! to findout when sending finished after stopping
        connect(this, &Scheme::sendRelayIsRunning, &loop, [&loop](bool sending) {
            if (!sending)
                loop.exit(AppSpecCPP::ChangeType::ctNone);
        });
    }

    if (timeout == 0) {
        return 0;
    } else if (timeout > 0) {
        // quit will exit with, same as exit(ChangeType::CurrentTemperature)
        QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
    }

    return loop.exec();
}

//TODO
void Scheme::updateVacationState()
{
    if (stopWork)
        return;

    if (mRealSysMode != AppSpecCPP::SystemMode::Vacation)
        return; // we can also assert as this should not happen

    SCHEME_LOG << "mCurrentSysMode " << mCurrentSysMode;
    AppSpecCPP::SystemMode realSysMode = AppSpecCPP::SystemMode::Off;

    if (mCurrentSysMode == AppSpecCPP::SystemMode::Cooling) {
        if (mDataProvider.data()->currentTemperature() > effectiveTemperature() - STAGE1_OFF_RANGE) { // before stage 1 off
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        } else if (mDataProvider.data()->currentTemperature() > effectiveTemperature() - STAGE1_ON_RANGE) { // before stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Off;
        } else {  // stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Heating;
        }
    } else if (mCurrentSysMode == AppSpecCPP::SystemMode::Heating) {
        if (mDataProvider.data()->currentTemperature() < effectiveTemperature() + STAGE1_OFF_RANGE) { // before stage 1 off
            realSysMode = AppSpecCPP::SystemMode::Heating;
        } else if (mDataProvider.data()->currentTemperature() < effectiveTemperature() + STAGE1_ON_RANGE) { // before stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Off;
        } else {  // stage 1 on
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        }
    } else { // OFF
        if (mDataProvider.data()->currentTemperature() < effectiveTemperature() - STAGE1_ON_RANGE) {
            realSysMode = AppSpecCPP::SystemMode::Heating;
        } else if (mDataProvider.data()->currentTemperature() > effectiveTemperature() + STAGE1_ON_RANGE) {
            realSysMode = AppSpecCPP::SystemMode::Cooling;
        } else {
            // what should we do here?
        }
    }

    if (mVacationMinimumTemperature > mDataProvider.data()->currentTemperature()) {
        realSysMode = AppSpecCPP::SystemMode::Heating;
        //        range = temperature - $current_state['min_temp'];
    } else if (mVacationMaximumTemperature < mDataProvider.data()->currentTemperature()) {
        realSysMode = AppSpecCPP::SystemMode::Cooling;
        //        range = temperature - current_state['max_temp'];
    }

    // Update current system mode
    mCurrentSysMode = realSysMode;
    mRealSysMode = realSysMode;
}

void Scheme::moveToUpdatingMode()
{
    mRestarting = true;

    // If the system fails to restart within 10 seconds, it returns to normal operation.
    mUpdatingTimer.start(10000);
}

double Scheme::effectiveTemperature()
{
    return mDataProvider->effectiveTemperature();
}

double Scheme::effectiveCurrentTemperature()
{
    return mDataProvider->currentTemperature();
}

AppSpecCPP::FanMode Scheme::fanMode() const {
    return mFanMode;
}

void Scheme::runSystemDelay(AppSpecCPP::SystemMode mode)
{
    auto sysDelay = mDataProvider->systemSetup()->systemRunDelay * 60000;

    emit startSystemDelayCountdown(mode, sysDelay);
    waitLoop(sysDelay, AppSpecCPP::ctMode);
    emit stopSystemDelayCountdown();
}

void Scheme::setVacation()
{
    auto vacation = mDataProvider.data()->vacation();
    mVacationMinimumTemperature = UtilityHelper::toFahrenheit(vacation.minimumTemperature);
    mVacationMaximumTemperature = UtilityHelper::toFahrenheit(vacation.maximumTemperature);
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

    if (isOn) {
        mFanWPHTimer.start(mFanWPH * 60 * 1000);

    } else {
        // Move to auto mode
        mFanWPHTimer.stop();
    }

    mRelay->setFanMode(isOn);

    sendRelays();
}

void Scheme::checkForRestart()
{
    const auto sys = mDataProvider.data()->systemSetup();
    if (sys && sys->systemType == AppSpecCPP::SystemType::DualFuelHeating &&
        (mActiveSysTypeHeating == AppSpecCPP::HeatPump ||
         mActiveSysTypeHeating == AppSpecCPP::HeatingOnly)) {
        auto activeType = activeSystemTypeHeating();
        if (activeType != mActiveSysTypeHeating) {
            SCHEME_LOG << "Restart scheme due to dual fuel change." << mActiveSysTypeHeating << activeType << sys->systemMode;
            LOG_CHECK_SCHEME(mDataProvider->isPerfTestRunning()) << "Restart scheme due to dual fuel change." << mDataProvider->effectiveSystemMode();
            restartWork();
        }
    }
}

void Scheme::setSystemSetup()
{
    const auto sys = mDataProvider.data()->systemSetup();
    // ob_state_ initial value?
    mRelay->setOb_on_state((sys->systemType == AppSpecCPP::SystemType::HeatPump || sys->systemType == AppSpecCPP::SystemType::DualFuelHeating) ?
                               (sys->heatPumpOBState == 0 ? AppSpecCPP::Cooling
                                                          : AppSpecCPP::Heating) :
                               AppSpecCPP::Off);

    connect(sys, &SystemSetup::systemModeChanged, this, [=] {
        SCHEME_LOG << "systemModeChanged: " << sys->systemMode;
        LOG_CHECK_SCHEME(mDataProvider->isPerfTestRunning())
            << "Effective system-mode: " << mDataProvider->effectiveSystemMode();

        restartWork();
    });

    connect(sys, &SystemSetup::isVacationChanged, this, [=] {
        SCHEME_LOG<< "isVacationChanged: "<< sys->isVacation;
        LOG_CHECK_SCHEME(mDataProvider->isPerfTestRunning())<< "isVacationChanged: "<< mDataProvider->isVacationEffective();

        restartWork();
    });

    connect(sys, &SystemSetup::systemTypeChanged, this, [=] {
        SCHEME_LOG<< "systemTypeChanged: "<< sys->systemType << sys->heatPumpOBState;

        // CHECK for dual fuel
        mRelay->setOb_on_state((sys->systemType == AppSpecCPP::SystemType::HeatPump || sys->systemType == AppSpecCPP::SystemType::DualFuelHeating) ?
                                   (sys->heatPumpOBState == 0 ? AppSpecCPP::Cooling
                                                              : AppSpecCPP::Heating) :
                                   AppSpecCPP::Off);

        // Use automatic mode in dual fuel heating when system type change
        switchDFHActiveSysType(AppSpecCPP::SysTUnknown);

        restartWork();
    });

    // these parameters will be used in control loop, if any condition locked to these update here
    connect(sys, &SystemSetup::heatPumpOBStateChanged, this, [=] {
        auto isHeatPumpAvailable = sys->systemType == AppSpecCPP::SystemType::HeatPump || sys->systemType == AppSpecCPP::SystemType::DualFuelHeating;
        if (isHeatPumpAvailable)
            SCHEME_LOG << "heatPumpOBStateChanged: " << sys->heatPumpOBState << sys->systemType;
        mRelay->setOb_on_state(isHeatPumpAvailable ?
                                   (sys->heatPumpOBState == 0 ? AppSpecCPP::Cooling
                                                              : AppSpecCPP::Heating) :
                                   AppSpecCPP::Off);
    });

    connect(sys, &SystemSetup::coolStageChanged, this, [=] {
        if (sys->systemType != AppSpecCPP::SystemType::HeatingOnly)
            SCHEME_LOG << "coolStageChanged: " << sys->coolStage;
    });
    connect(sys, &SystemSetup::heatStageChanged, this, [=] {
        if (sys->systemType != AppSpecCPP::SystemType::CoolingOnly &&
            sys->systemType != AppSpecCPP::SystemType::HeatPump)
            SCHEME_LOG << "heatStageChanged: " << sys->heatStage;
    });

    // Just log the change
    connect(sys, &SystemSetup::systemRunDelayChanged, this, [=] {
        SCHEME_LOG << "systemRunDelayChanged: " << sys->systemRunDelay << sys->systemType;
    });

    connect(sys, &SystemSetup::dualFuelThresholdChanged, this, [=] {
        SCHEME_LOG << "dualFuelThresholdChanged" << mActiveSysTypeHeating << sys->systemMode;
        LOG_CHECK_SCHEME(mDataProvider->isPerfTestRunning()) << "dualFuelThresholdChanged" << mDataProvider->effectiveSystemMode();
        // restart scheme if needed
        if (sys->systemType == AppSpecCPP::SystemType::DualFuelHeating && sys->isAUXAuto) {
            checkForRestart();
        }
    });

    connect(sys, &SystemSetup::isAUXAutoChanged, this, [=] {
        SCHEME_LOG << "isAUXAutoChanged";
        // restart scheme if needed
        if (sys->systemType == AppSpecCPP::SystemType::DualFuelHeating)
            checkForRestart();
    });

    connect(sys, &SystemSetup::dualFuelManualHeatingChanged, this, [=] {
        SCHEME_LOG << "dualFuelManualHeatingChanged" << sys->dualFuelManualHeating;
        // restart scheme if needed
        if (mDataProvider->effectiveSystemMode() != AppSpecCPP::Auto && !mDataProvider->isVacationEffective() &&
            sys->systemType == AppSpecCPP::SystemType::DualFuelHeating && !sys->isAUXAuto)
            checkForRestart();
    });

    connect(sys, &SystemSetup::dualFuelHeatingModeDefaultChanged, this, [=] {
        SCHEME_LOG << "dualFuelHeatingModeDefaultChanged" << sys->dualFuelHeatingModeDefault;
        // restart scheme if needed
        if ((mDataProvider->effectiveSystemMode() == AppSpecCPP::Auto || mDataProvider->isVacationEffective()) &&
            sys->systemType == AppSpecCPP::SystemType::DualFuelHeating && !sys->isAUXAuto)
            checkForRestart();
    });

    connect(sys, &SystemSetup::auxiliaryHeatingChanged, this, [=] {
        SCHEME_LOG << "auxiliaryHeating changed to " << sys->auxiliaryHeating;
        if (sys->systemType == AppSpecCPP::SystemType::HeatPump)
            restartWork();
    });

    connect(sys, &SystemSetup::useAuxiliaryParallelHeatPumpChanged, this, [=] {
        SCHEME_LOG << "useAuxiliaryParallelHeatPump changed to " << sys->useAuxiliaryParallelHeatPump;
        if (sys->systemType == AppSpecCPP::SystemType::HeatPump)
            restartWork();
    });

    connect(sys, &SystemSetup::driveAux1AndETogetherChanged, this, [=] {
        SCHEME_LOG << "driveAux1AndETogether changed to " << sys->driveAux1AndETogether;
        if (sys->systemType == AppSpecCPP::SystemType::HeatPump)
            restartWork();
    });

    connect(sys, &SystemSetup::driveAuxAsEmergencyChanged, this, [=] {
        SCHEME_LOG << "driveAuxAsEmergency changed to " << sys->driveAuxAsEmergency;
        if (sys->systemType == AppSpecCPP::SystemType::HeatPump && mDataProvider->effectiveSystemMode() == AppSpecCPP::SystemMode::EmergencyHeat)
            restartWork();
    });
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
