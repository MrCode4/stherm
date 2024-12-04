#include "HumidityScheme.h"
#include "LogCategoires.h"
#include "SchemeDataProvider.h"

#include <QTimer>

HumidityScheme::HumidityScheme(DeviceAPI* deviceAPI, QSharedPointer<SchemeDataProvider> schemeDataProvider, QObject *parent) :
    BaseScheme(deviceAPI, schemeDataProvider, parent)
{
}

HumidityScheme::~HumidityScheme()
{
    stop();
}

void HumidityScheme::stop()
{
    LOG_SCHEME << "stopping HVAC (Humidity control)" ;

    stopWork = true;
    emit stopWorkRequested();

    // Stop worker.
    terminate();
    LOG_SCHEME << "terminated HVAC (Humidity control)" ;
    wait();

    LOG_SCHEME << "stopped HVAC (Humidity control)" ;
}

void HumidityScheme::run()
{
    LOG_SCHEME << "-- startWork is running fro Humidity control." << QThread::currentThreadId();

    if (!mDataProvider.data()->systemSetup()) {
        LOG_SCHEME << "-- SystemSetup is not ready.";
        return;
    }

    while (!stopWork) {
        // If no wire go to offloop

        auto sysSetup = mDataProvider->systemSetup();
        if (sysSetup->systemAccessories->getAccessoriesWireType() == AppSpecCPP::None ||
            mDataProvider.data()->effectiveSystemMode() == AppSpecCPP::SystemMode::Off) {
            OffLoop();

            if (stopWork)
                break;

            continue;
        }

        // Vacation has a higher priority compared to other processes.
        if (mDataProvider.data()->isVacationEffective()) {
            VacationLoop();

        } else {
            normalLoop();

        }

        // all should be off
        // Turn off the humidity relays
        turnOffAccessoriesRelays();
        sendRelays(true);
        if (stopWork)
            break;

        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
    }
}

void HumidityScheme::restartWork(bool forceStart)
{
    if (isRunning()) {
        LOG_SCHEME << "restarting Humidity scheme" << stopWork;
        if (stopWork) // restart is already in progress
            return;
        // Any finished signal should not start the worker.
        connect(
            this,
            &HumidityScheme::finished,
            this,
            [=]() {
                LOG_SCHEME << "restarted Humidity scheme";
                stopWork = false;
                // mLogTimer.start();
                this->start();
            },
            Qt::SingleShotConnection);

        stopWork = true;
        emit stopWorkRequested();
        this->wait(QDeadlineTimer(1000, Qt::PreciseTimer));

    } else if (forceStart){
        LOG_SCHEME << "started Humidity scheme";
        stopWork = false;
        // mLogTimer.start();
        this->start();

    } else {
        LOG_SCHEME << "trying to start before main start";
    }
}

void HumidityScheme::setSystemSetup()
{
    const auto sys = mDataProvider.data()->systemSetup();

    connect(sys, &SystemSetup::systemModeChanged, this, [=] {
        LOG_SCHEME<< "systemModeChanged: "<< sys->systemMode;
        SCHEME_LOG_CHECK(mDataProvider->isPerfTestRunning())<< "Effective system-mode: "<< mDataProvider->effectiveSystemMode();

        restartWork();
    });

    connect(sys, &SystemSetup::isVacationChanged, this, [=] {
        LOG_SCHEME<< "isVacationChanged: "<< mDataProvider.data()->isVacationEffective();

        restartWork();
    });

    connect(sys->systemAccessories, &SystemAccessories::accessoriesChanged, this, [=] {
        LOG_SCHEME<< "Accessories Type: "<< mDataProvider->getAccessoriesType();


        LOG_SCHEME<< "Accessories Wire Type: "<< mDataProvider->getAccessoriesWireType();

        if (mDataProvider->getAccessoriesWireType() == AppSpecCPP::None) {
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
    auto lastConfigs = mRelay->relaysLast();

    if (mDataProvider->isRelaysInitialized() &&
        (lastConfigs == relaysConfig)) {
        SCHEME_LOG_CHECK(false) << "no change";
        return;
    }

    if (!mCanSendRelay) {
        waitLoop(-1, AppSpecCPP::ctSendRelay);
    }

    // To ensure the temperature relays updated.
    mRelay->setRelaysLast(relaysConfig);

    emit sendRelayIsRunning(true);

    if (!mDataProvider->isRelaysInitialized()) {
        // Send the last relays
        emit updateRelays(relaysConfig, true);
        waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
    }

    mDataProvider->setIsRelaysInitialized(true);

    if (debugMode) {
        auto steps = lastConfigs.changeStepsSorted(relaysConfig);
        for (int var = 0; var < steps.size(); var++) {
            auto step = steps.at(var);
            LOG_SCHEME << step.first.c_str() << step.second;
            if (step.first == "g"){
                lastConfigs.g = relaysConfig.g;
                LOG_SCHEME << relaysConfig.g;

            } else if (step.first == "acc2"){
                lastConfigs.acc2 = relaysConfig.acc2;
                LOG_SCHEME << relaysConfig.acc2;

            } else if (step.first == "acc1p"){
                lastConfigs.acc1p = relaysConfig.acc1p;
                LOG_SCHEME << relaysConfig.acc1p;

            } else if (step.first == "acc1n"){
                lastConfigs.acc1n = relaysConfig.acc1n;
                LOG_SCHEME << relaysConfig.acc1n << relaysConfig.acc1p;

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

    SCHEME_LOG_CHECK(false) << "finished";

    emit sendRelayIsRunning(false);
}

void HumidityScheme::VacationLoop()
{
    // In vacation range
    if (checkVacationRange()) {
        LOG_SCHEME << "VacationLoop: Vacation in range";
        turnOffAccessoriesRelays();
        return;
    }

    LOG_SCHEME << "Start VacationLoop, AccessoriesType: " << mDataProvider->getAccessoriesType() <<
        " - mVacationMinimumHumidity" << mVacationMinimumHumidity <<
        " - mVacationMaximumHumidity" << mVacationMaximumHumidity <<
        " - currentHumidity" <<mDataProvider.data()->currentHumidity();

    if (mDataProvider->getAccessoriesType() == AppSpecCPP::AccessoriesType::Humidifier) {

        if ((mVacationMinimumHumidity - mDataProvider.data()->currentHumidity()) > 0.001) {
            // Humidity loop
            while ((mDataProvider.data()->currentHumidity() - mVacationMaximumHumidity) < 0.001) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                // Set off the humidity relays in cooling mode
                if (mRelay->currentState() == AppSpecCPP::SystemMode::Cooling)
                    break;

                updateAccessoriesRelays(mDataProvider->getAccessoriesWireType());
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }

        // Dehumidifiers can only reduce humidity, and may not be able to consistently maintain the desired vacation humidity range.
        // you can be confident that a dehumidifier will always lower the humidity to mVacationMaximumHumidity
    } else if (mDataProvider->getAccessoriesType() == AppSpecCPP::AccessoriesType::Dehumidifier) {
        if (mDataProvider.data()->currentHumidity() - mVacationMaximumHumidity > 0.001) {

            // Dehumidifier loop
            // mVacationMinimumHumidity < mDataProvider.data()->currentHumidity() Just  check but not important
            while (mVacationMinimumHumidity - mDataProvider.data()->currentHumidity() < 0.001) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                updateAccessoriesRelays(mDataProvider->getAccessoriesWireType());
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }
    }

    LOG_SCHEME << "END VacationLoop, current relay state:" << mRelay->currentState();
}

bool HumidityScheme::checkVacationRange() {
    auto currentHumidity = mDataProvider.data()->currentHumidity();
    return (mVacationMaximumHumidity - currentHumidity) > 0.001 &&
           (mVacationMinimumHumidity - currentHumidity) < 0.001;
}

void HumidityScheme::normalLoop()
{
    LOG_SCHEME << "AccessoriesType: " << mDataProvider->getAccessoriesType() <<
        " - currentHumidity: " << mDataProvider->currentHumidity() <<
        " - effectiveSetHumidity: " << effectiveSetHumidity();

    if (mDataProvider->getAccessoriesType() == AppSpecCPP::AccessoriesType::Dehumidifier) {

        if (mDataProvider.data()->currentHumidity() > effectiveSetHumidity()) {

            while (effectiveSetHumidity() - mDataProvider.data()->currentHumidity() < 10) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                updateAccessoriesRelays(mDataProvider->getAccessoriesWireType());
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }

    } else if (mDataProvider->getAccessoriesType() == AppSpecCPP::AccessoriesType::Humidifier) {

        if (mDataProvider.data()->currentHumidity() < effectiveSetHumidity()) {

            while (mDataProvider.data()->currentHumidity() - effectiveSetHumidity() < 10) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                // Set off the humidity relays in cooling mode
                if (mRelay->currentState() == AppSpecCPP::SystemMode::Cooling)
                    break;

                updateAccessoriesRelays(mDataProvider->getAccessoriesWireType());

                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }

    } else {
        LOG_SCHEME << "Wrong Accessories Type" << mDataProvider->getAccessoriesType();
    }

    LOG_SCHEME << "END normalLoop, current relay state:" << mRelay->currentState();
}

void HumidityScheme::setVacation()
{
    auto vacation = mDataProvider.data()->vacation();
    mVacationMinimumHumidity = vacation.minimumHumidity;
    mVacationMaximumHumidity = vacation.maximumHumidity;
}

void HumidityScheme::updateAccessoriesRelays(AppSpecCPP::AccessoriesWireType accessoriesWireType)
{
    mRelay->updateHumidityWiring(accessoriesWireType);

    sendRelays();
}

void HumidityScheme::turnOffAccessoriesRelays()
{
    mRelay->updateHumidityWiring(AppSpecCPP::None);

    sendRelays();
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
