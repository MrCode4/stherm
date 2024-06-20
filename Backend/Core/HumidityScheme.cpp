#include "HumidityScheme.h"
#include "LogHelper.h"
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

    if (!mDataProvider.data()->systemSetup()) {
        TRACE << "-- SystemSetup is not ready.";
        return;
    }

    while (!stopWork) {
        // Vacation has a higher priority compared to other processes.
        if (mDataProvider.data()->systemSetup()->isVacation) {
            VacationLoop();

        } else if (mDataProvider.data()->systemSetup()->systemMode == AppSpecCPP::SystemMode::Off) {
            OffLoop();

        } else {
            normalLoop();

        }

        turnOffAccessoriesRelays();
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

void HumidityScheme::setSystemSetup()
{
    const auto sys = mDataProvider.data()->systemSetup();

    connect(sys, &SystemSetup::systemModeChanged, this, [=] {
        TRACE<< "systemModeChanged: "<< sys->systemMode;

        restartWork();
    });

    connect(sys, &SystemSetup::isVacationChanged, this, [=] {
        TRACE<< "isVacationChanged: "<< sys->isVacation;

        restartWork();
    });

    connect(sys->systemAccessories, &SystemAccessories::accessoriesChanged, this, [=] {
        mAccessoriesType = sys->systemAccessories->getAccessoriesType();
        TRACE<< "Accessories Type: "<< mAccessoriesType;


        mAccessoriesWireType = sys->systemAccessories->getAccessoriesWireType();
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
        turnOffAccessoriesRelays();
        return;
    }

    if (mAccessoriesType == AppSpecCPP::AccessoriesType::Humidifier) {

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

                updateAccessoriesRelays(mAccessoriesWireType);
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }

        // Dehumidifiers can only reduce humidity, and may not be able to consistently maintain the desired vacation humidity range.
        // you can be confident that a dehumidifier will always lower the humidity to mVacationMaximumHumidity
    } else if (mAccessoriesType == AppSpecCPP::AccessoriesType::Dehumidifier) {
        if (mDataProvider.data()->currentHumidity() - mVacationMaximumHumidity > 0.001) {

            // Dehumidifier loop
            // mVacationMinimumHumidity < mDataProvider.data()->currentHumidity() Just  check but not important
            while (mVacationMinimumHumidity - mDataProvider.data()->currentHumidity() < 0.001) {
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
    auto currentHumidity = mDataProvider.data()->currentHumidity();
    return (mVacationMaximumHumidity - currentHumidity) > 0.001 &&
           (mVacationMinimumHumidity - currentHumidity) < 0.001;
}

void HumidityScheme::normalLoop()
{
    if (mAccessoriesType == AppSpecCPP::AccessoriesType::Dehumidifier) {

        if (mDataProvider.data()->currentHumidity() > effectiveSetHumidity()) {

            while (effectiveSetHumidity() - mDataProvider.data()->currentHumidity() < 10) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                updateAccessoriesRelays(mAccessoriesWireType);
                waitLoop(RELAYS_WAIT_MS, AppSpecCPP::ctNone);
            }
        }

    } else if (mAccessoriesType == AppSpecCPP::AccessoriesType::Humidifier) {

        if (mDataProvider.data()->currentHumidity() < effectiveSetHumidity()) {

            while (mDataProvider.data()->currentHumidity() - effectiveSetHumidity() < 10) {
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

    } else {
        TRACE << "Wrong Accessories Type";
    }

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
