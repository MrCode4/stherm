//
// Created by Mortie on 11/2/2023.
//

#ifndef HVAC_DAEMON_TITHREAD_H
#define HVAC_DAEMON_TITHREAD_H

#include <QThread>

#include "Peripheral.h"
extern char dev_id[16];
constexpr char Daemon_Version[] = "01.01";

/**
* @brief Function to handle updates.
*
* @param sig The signal received.
*/
void update_prepare(int sig);

class TIThread : public QThread
{
    Q_OBJECT

public:
    explicit TIThread(void *a, QObject *parent = nullptr);

    ~TIThread() override;

    static int set_update_ti;
    std::string TI_SW;
    std::string TI_HW;
    std::string NRF_SW;
    std::string NRF_HW;

protected:
    void run() override;

private:
    thread_Data ti;

    bool m_initialized = false;
};

#endif //HVAC_DAEMON_TITHREAD_H
