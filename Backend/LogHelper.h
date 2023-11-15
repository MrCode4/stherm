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

#ifdef _MSC_VER
#define __METHOD__ __FUNCTION__
#elif __GNUC__
//#define __METHOD__ methodName(__PRETTY_FUNCTION__)
#define __METHOD__ __PRETTY_FUNCTION__
#else
#define __METHOD__ __func__
#error please define a PACK macro for your compiler
#endif

#define TRACE \
    qDebug() << QString("%0 - %1  Method: %2 Line: %3  Message: %4") \
                    .arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")) \
                    .arg(__FILE__) \
                    .arg(__METHOD__) \
                    .arg(__LINE__)

#define LOG_DEBUG(message) Logger::logDebug(message, __FILE__, __LINE__)

// Example usage:
// LOG_DEBUG("This is a debug message");
