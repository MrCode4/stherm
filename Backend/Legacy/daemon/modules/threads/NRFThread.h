//
// Created by Mortie on 11/1/2023.
//

#ifndef HVAC_DAEMON_NRFTHREAD_H
#define HVAC_DAEMON_NRFTHREAD_H

#include <QThread>

#include "Peripheral.h"

class NRFThread : public QThread
{
    Q_OBJECT

public:
    explicit NRFThread(void *a, QObject *parent = nullptr);
    ~NRFThread() override;

protected:
    void run() override;

    bool m_initialized = false;

    thread_Data nrf;
};


#endif //HVAC_DAEMON_NRFTHREAD_H
