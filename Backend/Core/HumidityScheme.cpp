#include "HumidityScheme.h"
#include "LogHelper.h"

#include <QTimer>

HumidityScheme::HumidityScheme(DeviceAPI* deviceAPI, QObject *parent) :
    BaseScheme(deviceAPI, parent)
{
}

HumidityScheme::~HumidityScheme()
{
    stop();
}

void HumidityScheme::stop()
{
    TRACE << "stopping HVAC (Humidity control)" ;

    stopWork = true;
    emit stopWorkRequested();

    // Stop worker.
    terminate();
    TRACE << "terminated HVAC (Humidity control)" ;
    wait();

    TRACE << "stopped HVAC (Humidity control)" ;
}

void HumidityScheme::run()
{
    TRACE << "-- startWork is running fro Humidity control." << QThread::currentThreadId();

    if (!mSystemSetup) {
        TRACE << "-- mSystemSetup is not ready.";
        return;
    }

    while (!stopWork) {
        // Vacation has a higher priority compared to other processes.
        if (mSystemSetup->isVacation) {
            VacationLoop();

        } else if (mSystemSetup->systemMode == AppSpecCPP::SystemMode::Off) {
            OffLoop();

        } else {
            normalLoop();

        }

        updateAccessoriesRelays();
        sendRelays(true);
        if (stopWork)
            break;

        // all should be off! we can assert here
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
    }
}

void HumidityScheme::restartWork()
{
    if (isRunning()) {
        TRACE << "restarting Humidity scheme" << stopWork;
        if (stopWork) // restart is already in progress
            return;
        // Any finished signal should not start the worker.
        connect(
            this,
            &HumidityScheme::finished,
            this,
            [=]() {
                TRACE << "restarted Humidity scheme";
                stopWork = false;
                // mLogTimer.start();
                this->start();
            },
            Qt::SingleShotConnection);

        stopWork = true;
        emit stopWorkRequested();
        this->wait(QDeadlineTimer(1000, Qt::PreciseTimer));

    } else {
        TRACE << "started Humidity scheme";
        stopWork = false;
        // mLogTimer.start();
        this->start();
    }
}

void HumidityScheme::setSystemSetup(SystemSetup *systemSetup)
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

    connect(mSystemSetup->systemAccessories, &SystemAccessories::accessoriesChanged, this, [this] {
        mAccessoriesType = mSystemSetup->systemAccessories->getAccessoriesType();
        TRACE<< "Accessories Type: "<< mAccessoriesType;


        mAccessoriesWireType = mSystemSetup->systemAccessories->getAccessoriesWireType();
        TRACE<< "Accessories Wire Type: "<< mAccessoriesWireType;

        if (mAccessoriesWireType == AppSpecCPP::None) {
            stopWork = true;

        } else {
            restartWork();
        }
    });
}

void HumidityScheme::OffLoop()
{
    waitLoop(-1, AppSpecCPP::ctMode);
}

void HumidityScheme::sendRelays(bool forceSend)
{
    if (!forceSend && stopWork)
        return;

    auto relaysConfig = mRelay->relays();

    if (lastConfigs == relaysConfig) {
        TRACE_CHECK(false) << "no change";
        return;
    }

    if (!mCanSendRelay) {
        waitLoop(-1, AppSpecCPP::ctSendRelay);
    }

    // Get changed relays, relays maybe changed in the Temperature scheme.
    relaysConfig = mRelay->relays();

    emit sendRelayIsRunning(true);

    if (debugMode) {
        auto steps = lastConfigs.changeStepsSorted(relaysConfig);
        for (int var = 0; var < steps.size(); var++) {
            auto step = steps.at(var);
            TRACE << step.first.c_str() << step.second;
            if (step.first == "g"){
                lastConfigs.g = relaysConfig.g;
                TRACE << relaysConfig.g;

            } else if (step.first == "acc2"){
                lastConfigs.acc2 = relaysConfig.acc2;
                TRACE << relaysConfig.acc2;

            } else if (step.first == "acc1p"){
                lastConfigs.acc1p = relaysConfig.acc1p;
                TRACE << relaysConfig.acc1p;

            } else if (step.first == "acc1n"){
                lastConfigs.acc1n = relaysConfig.acc1n;
                TRACE << relaysConfig.acc1n << relaysConfig.acc1p;

            } else {
                // To ignore Temperature relays
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

    // To ensure the temperature relays updated.
    lastConfigs = relaysConfig;
    TRACE_CHECK(false) << "finished";

    emit sendRelayIsRunning(false);
}

void HumidityScheme::VacationLoop()
{
    // In vacation range
    if (checkVacationRange()) {
        updateAccessoriesRelays();
        return;
    }

    if (mAccessoriesType == AppSpecCPP::AccessoriesType::Humidifier) {

        if ((mVacationMinimumHumidity - mCurrentHumidity) > 0.001) {
            // Humidity loop
            while ((mCurrentHumidity - mVacationMaximumHumidity) < 0.001) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                // Set off the humidity relays in cooling mode
                if (mRelay->currentState() == AppSpecCPP::SystemMode::Cooling)
                    break;

                updateAccessoriesRelays(mAccessoriesWireType);
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }

        // Dehumidifiers can only reduce humidity, and may not be able to consistently maintain the desired vacation humidity range.
        // you can be confident that a dehumidifier will always lower the humidity to mVacationMaximumHumidity
    } else if (mAccessoriesType == AppSpecCPP::AccessoriesType::Dehumidifier) {
        if (mCurrentHumidity - mVacationMaximumHumidity > 0.001) {

            // Dehumidifier loop
            // mVacationMinimumHumidity < mCurrentHumidity Just  check but not important
            while (mVacationMinimumHumidity - mCurrentHumidity < 0.001) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                updateAccessoriesRelays(mAccessoriesWireType);
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }
    }
}

bool HumidityScheme::checkVacationRange() {
    return (mVacationMaximumHumidity - mCurrentHumidity) > 0.001 &&
           (mVacationMinimumHumidity - mCurrentHumidity) < 0.001;
}

void HumidityScheme::normalLoop()
{
    if (mAccessoriesType == AppSpecCPP::AccessoriesType::Dehumidifier) {

        while (mCurrentHumidity - effectiveHumidity() > 10) {
            // Exit from loop
            if (stopWork) {
                break;
            }

            updateAccessoriesRelays(mAccessoriesWireType);
            waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
        }

    } else if (mAccessoriesType == AppSpecCPP::AccessoriesType::Humidifier) {

        while (mCurrentHumidity - effectiveHumidity() < 10) {
            // Exit from loop
            if (stopWork) {
                break;
            }

            // Set off the humidity relays in cooling mode
            if (mRelay->currentState() == AppSpecCPP::SystemMode::Cooling)
                break;

            updateAccessoriesRelays(mAccessoriesWireType);

            waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
        }

    } else {
        TRACE << "Wrong Accessories Type";
    }

}

void HumidityScheme::setVacation(const STHERM::Vacation &newVacation)
{
    mVacationMinimumHumidity = newVacation.minimumHumidity;
    mVacationMaximumHumidity = newVacation.maximumHumidity;
}

void HumidityScheme::setRequestedHumidity(const double &setPointHumidity)
{
    if (qAbs(mSetPointHumidity - setPointHumidity) < 0.001) {
        return;
    }

    mSetPointHumidity = setPointHumidity;
}

void HumidityScheme::updateAccessoriesRelays(AppSpecCPP::AccessoriesWireType accessoriesWireType)
{
    mRelay->updateHumidityWiring(accessoriesWireType);

    sendRelays();
}

double HumidityScheme::effectiveHumidity()
{
    double effHumidity = mSetPointHumidity;

    if (mSystemSetup->isVacation) {
        if ((mVacationMinimumHumidity - mCurrentHumidity) > 0.001) {
            effHumidity  = mVacationMinimumHumidity;

        } else if ((mVacationMaximumHumidity - mCurrentHumidity) < 0.001) {
            effHumidity  = mVacationMaximumHumidity;
        }

    } else if (mSchedule) {
        effHumidity  = mSchedule->humidity;

    }

    return effHumidity ;
}

int HumidityScheme::waitLoop(int timeout, AppSpecCPP::ChangeTypes overrideModes)
{
    QEventLoop loop;
    // connect signal for handling stopWork
    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctCurrentHumidity)){
        connect(this, &HumidityScheme::currentHumidityChanged, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctCurrentHumidity);
        });
    }

    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctSetHumidity)){
        connect(this, &HumidityScheme::setHumidityChanged, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctSetHumidity);
        });
    }

    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctMode)){
        connect(this, &HumidityScheme::stopWorkRequested, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctMode);
        });
    }

    if (overrideModes.testFlag(AppSpecCPP::ChangeType::ctSendRelay)){
        connect(this, &HumidityScheme::canSendRelay, &loop, [&loop]() {
            loop.exit(AppSpecCPP::ChangeType::ctSendRelay);
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
