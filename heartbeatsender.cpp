#include "heartbeatsender.h"

#include "LogHelper.h"

HeartbeatSender::HeartbeatSender(QObject *parent)
    : QObject{parent},
    watchdogPath_("./watchdog/watchdog"),
    heartbeatTimer_(std::make_unique<QTimer>(this))
{
    setConnections();
}

HeartbeatSender::~HeartbeatSender()
{
    sharedMemory_.detach();
}

void HeartbeatSender::setConnections()
{
    connect(heartbeatTimer_.get(), &QTimer::timeout, this, &HeartbeatSender::sendHeartbeat, Qt::DirectConnection);
}

void HeartbeatSender::sendHeartbeat()
{
    if (sharedMemory_.lock()) {
        char* data = (char*)sharedMemory_.data();
        QString heartbeat = "heartbeat";
        memcpy(data, heartbeat.toStdString().c_str(), heartbeat.length() + 1);
        sharedMemory_.unlock();
    } else {
        qDebug() << "Failed to lock shared memory.";
    }
}

bool HeartbeatSender::init()
{
    if (!QFile::exists(watchdogPath_)) {
        TRACE << "Error: Watchdog executable does not exist at" << watchdogPath_;
        return false;
    }

    if (!watchdogProcess_.startDetached(watchdogPath_)) {
        TRACE << "Failed to start Watchdog process!";
        return false;
    }

    sharedMemory_.setKey("MainAppHeartbeat");

    if (!sharedMemory_.create(1024))
        sharedMemory_.attach();

    return true;
}

bool HeartbeatSender::runWatchdogProcess()
{
    if(!init())
        return false;

    heartbeatTimer_->start(1000);

    qDebug() << "Watchdog process started successfully.";

    return true;
}
