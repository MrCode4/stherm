#include "HumidityScheme.h"
#include "LogHelper.h"

HumidityScheme::HumidityScheme(DeviceAPI* deviceAPI, QObject *parent) :
    mDeviceAPI(deviceAPI),
    stopWork(true),
    QThread{parent}
{
    mRelay = Relay::instance();
}

HumidityScheme::~HumidityScheme()
{
    stop();
}

void HumidityScheme::stop()
{
    TRACE << "stopping HVAC (Humidity control)" ;

    stopWork = true;

    // Stop worker.
    terminate();
    TRACE << "terminated HVAC (Humidity control)" ;
    wait();

    TRACE << "stopped HVAC (Humidity control)" ;
}

void HumidityScheme::run()
{
    // Vacation has a higher priority compared to other processes.
    if (mSystemSetup->isVacation) {
        VacationLoop();

    } else {
        normalLoop();

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
        // emit stopWorkRequested();
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

    connect(mSystemSetup, &SystemSetup::systemModeChanged, this, [this] {
        TRACE<< "systemModeChanged: "<< mSystemSetup->systemMode;

        //! Maybe neet for off mode
        // restartWork();
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

void HumidityScheme::VacationLoop()
{
    // In vacation range
    if (checkVacationRange()) {
        updateRelays();
        return;
    }

    if (mAccessoriesType == AppSpecCPP::AccessoriesType::Humidifier) {

        if ((mVacationMinimumHumidity - mCurrentHumidity) > 0.001) {
            // Set off the humidity relays in cooling mode
            if (mRelay->currentState() == AppSpecCPP::SystemMode::Cooling) {
                updateRelays();

            } else {

                // Humidity loop
                while ((mVacationMaximumHumidity - mCurrentHumidity) > 0.001 && (mVacationMinimumHumidity - mCurrentHumidity) > 0.001) {
                    // Exit from loop
                    if (stopWork) {
                        break;
                    }

                    updateRelays(mAccessoriesWireType);
                }

                // Exit from loop and Off the humidity wiring.
                if ((mVacationMaximumHumidity - mCurrentHumidity) < 0.001)
                    updateRelays();
            }


        }

        // Dehumidifiers can only reduce humidity, and may not be able to consistently maintain the desired vacation humidity range.
        // you can be confident that a dehumidifier will always lower the humidity to mVacationMaximumHumidity
    } else if (mAccessoriesType == AppSpecCPP::AccessoriesType::Dehumidifier) {
        // Dehumidifier loop
        // mVacationMinimumHumidity < mCurrentHumidity Just  check but not important
        while (mVacationMaximumHumidity < mCurrentHumidity && mVacationMinimumHumidity < mCurrentHumidity) {
            // Exit from loop
            if (stopWork) {
                break;
            }

            updateRelays(mAccessoriesWireType);
        }

        if (mVacationMaximumHumidity >= mCurrentHumidity && mVacationMinimumHumidity >= mCurrentHumidity) {
            updateRelays();
        }

    } else {
        updateRelays();
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

            updateRelays(mAccessoriesWireType);

        }

        if (mCurrentHumidity - effectiveHumidity() <= 10)
            updateRelays();

    } else if (mAccessoriesType == AppSpecCPP::AccessoriesType::Humidifier) {

        // Set off the humidity relays in cooling mode
        if (mRelay->currentState() == AppSpecCPP::SystemMode::Cooling) {
            updateRelays();

        } else {
            while (mCurrentHumidity - effectiveHumidity() < 10) {
                // Exit from loop
                if (stopWork) {
                    break;
                }

                updateRelays(mAccessoriesWireType);

            }

            if (mCurrentHumidity - effectiveHumidity() >= 10)
                updateRelays();
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

void HumidityScheme::setSchedule(ScheduleCPP *newSchedule)
{
    if (mSchedule == newSchedule)
        return;

    mSchedule = newSchedule;
}

void HumidityScheme::setRequestedHumidity(const double &setPointHumidity)
{
    if (qAbs(mSetPointHumidity - setPointHumidity) < 0.001) {
        return;
    }

    mSetPointHumidity = setPointHumidity;
}

void HumidityScheme::updateRelays(AppSpecCPP::AccessoriesWireType accessoriesWireType)
{
    mRelay->updateHumidityWiring(accessoriesWireType);
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
