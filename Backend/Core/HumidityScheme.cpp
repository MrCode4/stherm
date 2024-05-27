#include "HumidityScheme.h"
#include "LogHelper.h"

HumidityScheme::HumidityScheme(QObject *parent) :
    stopWork(true),
    QThread{parent}
{
    mRelay = Relay::instance();
}

void HumidityScheme::run()
{

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
        TRACE<< "systemAccessories changed: "<< mSystemSetup->isVacation;

        restartWork();
    });
}
