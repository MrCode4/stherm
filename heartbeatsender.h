#ifndef HEARTBEATSENDER_H
#define HEARTBEATSENDER_H

#include <QObject>
#include <QProcess>
#include <QSharedMemory>
#include <QTimer>

class HeartbeatSender : public QObject
{
    Q_OBJECT
public:
     HeartbeatSender(QObject *parent = nullptr);
    ~HeartbeatSender();
    //!
    //! \brief runWatchdogProcess
    //! try to send hearbeat each 1second to show that the main thread is alive
    //! \return
    //!
    bool runWatchdogProcess();

private:
    QProcess watchdogProcess_;
    QSharedMemory sharedMemory_;
    std::unique_ptr<QTimer> heartbeatTimer_;
    QString watchdogPath_;

    //!
    //! \brief init
    //! initialize the watchdog process and sharedMemory
    //! \return
    //!
    bool init();
    void setConnections();

private slots:
    void sendHeartbeat();
};

#endif // HEARTBEATSENDER_H
