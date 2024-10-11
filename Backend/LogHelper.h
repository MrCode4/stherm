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
    qDebug() << QString("%0 - %1  Method: %2 Line: %3 Log: ") \
                    .arg(QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss"), \
                         __FILE__, \
                         __METHOD__) \
                    .arg(__LINE__) \
                    .toStdString() \
                    .c_str()

#define TRACE_CAT(category) \
    qCDebug(category) << QString("%0 - %1  Method: %2 Line: %3 Log: ") \
                    .arg(QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss"), \
                         __FILE__, \
                         __METHOD__) \
                    .arg(__LINE__) \
                    .toStdString() \
                    .c_str()


#define TRACE_CHECK(check) if (check) TRACE

#define TRACE_CAT_CHECK(category, check) if (check) TRACE_CAT(category)


#define LOG_PRINT \
    qDebug() << QString("%0 - Log: ") \
                    .arg(QDateTime::currentDateTime().toString("dd MMM yyyy hh:mm:ss")) \
                    .toStdString() \
                    .c_str()

#define LOG_CHECK(check) if (check) LOG_PRINT

#define LOG_DEBUG(message) Logger::logDebug(message, __FILE__, __LINE__)

// Example usage:
// LOG_DEBUG("This is a debug message");
