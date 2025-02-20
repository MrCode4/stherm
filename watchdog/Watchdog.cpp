#include "Watchdog.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>
#include "LogHelper.h"

Watchdog::Watchdog(QObject *parent)
    : QObject(parent),
    sharedMemory_(new QSharedMemory("MainAppHeartbeat", this)),
    heartbeatTimer_(new QTimer(this)),
    memoryTimer_(new QTimer(this)),
    lastHeartbeatTime_(-1),
    laggyCounter_(0),
    timeoutCounter_(0)
{
    if (!initializeSharedMemory()) {
        qCritical() << "Failed to initialize shared memory. Exiting.";
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    setupConnections();

    heartbeatTimer_->start(1000); // Check heartbeat every second.
    memoryTimer_->start(5000);    // Check memory usage every 5 seconds.

    qDebug() << "Watchdog program started...";
}

Watchdog::~Watchdog()
{
    if (sharedMemory_->isAttached()) {
        sharedMemory_->detach();
    }
}

void Watchdog::setupConnections()
{
    connect(heartbeatTimer_, &QTimer::timeout, this, &Watchdog::onHeartbeatTimeout);
    connect(memoryTimer_, &QTimer::timeout, this, &Watchdog::onMemoryCheckTimeout);
}

bool Watchdog::initializeSharedMemory()
{
    if (!sharedMemory_->create(1024)) {
        if (!sharedMemory_->attach()) {
            return false;
        }
    }
    return true;
}

double Watchdog::calculateMemoryUsage() const
{
#ifdef __unix__
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open /proc/meminfo";
        return -1.0;
    }

    QTextStream in(&file);
    double totalMemory = 0;
    double availableMemory = 0;

    while (true) {
        QString line = in.readLine();
        if (line.isNull() || line.isEmpty())
            break;
        const QStringList parts = line.split(' ', Qt::SkipEmptyParts);
        if (parts.size() < 2)
            continue;
        if (parts[0] == "MemTotal:")
            totalMemory = parts[1].toDouble();
        else if (parts[0] == "MemAvailable:")
            availableMemory = parts[1].toDouble();
        if (totalMemory > 0 && availableMemory > 0)
            break;
    }

    return (totalMemory > 0) ? ((totalMemory - availableMemory) / totalMemory) * 100.0 : -1.0;
#else
    return -1.0;
#endif
}

void Watchdog::onMemoryCheckTimeout()
{
    const double memoryUsage = calculateMemoryUsage();
    if (memoryUsage < 0) {
        qWarning() << "Error reading memory usage!";
        return;
    }

    qDebug() << "Current Memory Usage:" << memoryUsage << "%";

    if (memoryUsage >= 95.0) {
        TRACE << "Memory usage is critical! Rebooting system...";
        memoryTimer_->stop();

        createWatchdogReport("Memory usage is critical!");
        //rebootSystem();
    }
}

void Watchdog::onHeartbeatTimeout()
{
    if (!sharedMemory_->lock()) {
        qDebug() << "Failed to lock shared memory!";
        return;
    }

    auto data = static_cast<char*>(sharedMemory_->data());
    const QString heartbeatMessage = QString::fromUtf8(data);

    if (heartbeatMessage == "heartbeat") {
        const qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

        if (lastHeartbeatTime_ != -1) {
            const qint64 interval = currentTime - lastHeartbeatTime_;
            if (interval > laggyInterval_) {
                qDebug() << "Delayed heartbeat detected:" << interval << "ms";
                ++laggyCounter_;
                if (laggyCounter_ >= laggyThreshold_) {
                    TRACE << "Persistent delayed heartbeat ("
                          << (laggyThreshold_ * 1.5)
                          << " seconds threshold reached). Rebooting system.";
                    heartbeatTimer_->stop();
                    sharedMemory_->unlock();

                    createWatchdogReport("Persistent delayed heartbeat detected!");
                    //rebootSystem();
                    return;
                }
            } else {
                qDebug() << "Heartbeat received. Interval:" << interval << "ms";
                laggyCounter_ = 0;
            }
        }

        lastHeartbeatTime_ = currentTime;
        timeoutCounter_ = 0;

        // Clear the heartbeat by writing a null terminator.
        data[0] = '\0';
        sharedMemory_->unlock();

    } else {
        sharedMemory_->unlock();

        ++timeoutCounter_;
        qDebug() << "No heartbeat received. Timeout count:" << timeoutCounter_;
        if (timeoutCounter_ >= timeoutThreshold_) {
            TRACE << "No heartbeat received for " << timeoutThreshold_
                  << " seconds. Rebooting system.";
            heartbeatTimer_->stop();

            createWatchdogReport("No heartbeat received!");
            //rebootSystem();

            return;
        }
    }

}

bool Watchdog::executeProcess(const QString &command, const QStringList &arguments,
                    QString &output, QString &errorOutput, int timeout)
{
    QProcess process;
    process.start(command, arguments);

    if (!process.waitForStarted()) {
        errorOutput = QString("Failed to start process '%1'.").arg(command);
        return false;
    }
    if (!process.waitForFinished(timeout)) {
        errorOutput = QString("Process '%1' did not finish in the allotted time.").arg(command);
        return false;
    }

    output = process.readAllStandardOutput();
    errorOutput = process.readAllStandardError();

    // Check if the process ended normally with a zero exit code.
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        errorOutput.append(QString(" Process '%1' exited abnormally with exit code %2.").arg(command).arg(process.exitCode()));
        return false;
    }

    return true;
}

void Watchdog::createWatchdogReport(const QString &reason)
{
#ifdef __unix__
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");//(watchdog_report_20250220_153045.log)
    QString filePath = QString("/mnt/log/watchdog_report_%1.log").arg(timestamp);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open log file:"
                   << filePath << ", error:" << file.errorString();
        return;
    }

    QTextStream out(&file);

    // Write header report generation time and the restart reason.
    out << "Watchdog Report generated at "
        << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
    out << "Restart Reason:\n" << reason << "\n\n";

    // Append CPU and Memory usage using the 'top' command.
    out << "CPU and Memory Usage:\n";
    QString topOutput, topError;
    bool topSuccess =
        executeProcess("top", QStringList() << "-b" << "-n1", topOutput, topError, 5000);
    if (!topSuccess) {
        out << "Error executing 'top': " << topError << "\n";
    } else {
        const QStringList topLines = topOutput.split('\n');
        for (const QString &line : topLines) {
            if (line.contains("Cpu(s):") || line.contains("Mem :"))
                out << line << "\n";
        }
    }
    out << "\n";

    // Append journalctl logs from the last 5 minutes.
    out << "Journalctl Logs (last 5 minutes):\n";
    QString journalOutput, journalError;
    bool journalSuccess = executeProcess("journalctl", QStringList() << "--no-pager" << "--since" << "5 minutes ago",
                                         journalOutput, journalError, 10000);
    if (!journalSuccess) {
        out << "Error executing 'journalctl': " << journalError << "\n";
    } else {
        out << journalOutput;
    }

    file.close();
    qDebug() << "Watchdog report saved to" << filePath;
#else
    //TODO::should be implemented for other OSs...
#endif
}

void Watchdog::rebootSystem()
{
    qDebug() << "Initiating system reboot...";

    if (!QProcess::startDetached("sudo", QStringList() << "reboot")) {
        qCritical() << "Failed to initiate system reboot.";
    }
}
