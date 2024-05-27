#pragma once

#include <QObject>
#include <QThread>

#include "Relay.h"
#include "SystemSetup.h"

/*! ***********************************************************************************************
 * This class controls the humidity.
 * ************************************************************************************************/

class HumidityScheme : public QThread
{
    Q_OBJECT
public:
    explicit HumidityScheme(QObject *parent = nullptr);

    void setSystemSetup(SystemSetup *systemSetup);

protected:
    void run() override;

signals:

private:
    //! Restart the worker thread
    void restartWork();

private:
    Relay*  mRelay;

    SystemSetup *mSystemSetup = nullptr;
     bool stopWork;
};

