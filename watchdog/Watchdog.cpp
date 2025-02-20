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
        rebootSystem();
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
                    rebootSystem();
                    sharedMemory_->unlock();
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
    } else {
        ++timeoutCounter_;
        qDebug() << "No heartbeat received. Timeout count:" << timeoutCounter_;
        if (timeoutCounter_ >= timeoutThreshold_) {
            TRACE << "No heartbeat received for " << timeoutThreshold_
                  << " seconds. Rebooting system.";
            heartbeatTimer_->stop();
            rebootSystem();
        }
    }

    sharedMemory_->unlock();
}

void Watchdog::rebootSystem()
{
    qDebug() << "Initiating system reboot...";

    if (!QProcess::startDetached("sudo", QStringList() << "reboot")) {
        qCritical() << "Failed to initiate system reboot.";
    }
}
