#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <QDateTime>
#include <QProcess>
#include <QObject>
#include <QSharedMemory>
#include <QTimer>

///
/// \brief The Watchdog class monitors heartbeat signals and system memory usage,
///        rebooting the system if anomalies are detected.
///
class Watchdog : public QObject
{
    Q_OBJECT
public:
    explicit Watchdog(QObject *parent = nullptr);
    ~Watchdog() override;

private slots:
    void onHeartbeatTimeout();
    void onMemoryCheckTimeout();

private:
    bool initializeSharedMemory();
    double calculateMemoryUsage() const;

    //!
    //! \brief createWatchdogReport
    //! Function to create a watchdog report log in /mnt/log/.
    //! \param reason
    //!
    void createWatchdogReport(const QString &reason);

    //!
    //! \brief executeProcess
    //! Helper function to execute a process with error handling.
    //! \param command
    //! \param arguments
    //! \param output
    //! \param errorOutput
    //! \param timeout
    //! \return true on success, false otherwise. Outputs are captured in 'output' and 'errorOutput'.
    //!
    bool executeProcess(const QString &command, const QStringList &arguments, QString &output, QString &errorOutput, int timeout = 10000);
    void rebootSystem();
    void setupConnections();    

    QSharedMemory* sharedMemory_;
    QTimer* heartbeatTimer_;
    QTimer* memoryTimer_;

    // Time and threshold tracking
    qint64 lastHeartbeatTime_;  // in milliseconds since epoch
    int laggyCounter_;
    const int laggyThreshold_{10};     // number of laggy intervals allowed
    const qint64 laggyInterval_{1200};  // maximum allowed interval (ms) between heartbeats
    int timeoutCounter_;
    const int timeoutThreshold_{10};   // number of missed heartbeats allowed
};

#endif // WATCHDOG_H
