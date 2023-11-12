#pragma once

#include <QDebug>
#include <QString>
#include <QFileInfo>

class Logger {
public:
    static void logDebug(const QString& message, const QString& file, int line) {
        QString logMessage = QString("%0 - %1  Line: %2  Message: %3").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"))
                                 .arg(file).arg(line).arg(message);
        qDebug().noquote() << logMessage;
    }
};

#define LOG_DEBUG(message) Logger::logDebug(message, __FILE__, __LINE__)

// Example usage:
// LOG_DEBUG("This is a debug message");
