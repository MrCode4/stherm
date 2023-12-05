#include "DateTimeManagerCPP.h"

#include <QTimeZone>

DateTimeManagerCPP::DateTimeManagerCPP(QObject *parent)
    : QObject{parent}
{}

bool DateTimeManagerCPP::isRunning() const
{
    return (mProcess.state() == QProcess::Starting || mProcess.state() == QProcess::Running);
}

void DateTimeManagerCPP::setAutoUpdateTime(bool autoUpdate)
{
    if (isRunning()) {
        return;
    }

    connect(&mProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus) {
            callProcessFinished({ exitCode });
        }, Qt::SingleShotConnection);

    mProcess.start("timedatectl", { "set-ntp", autoUpdate ? "true" : "false" });
}

QVariantList DateTimeManagerCPP::timezones() const
{
    QVariantList timezones;

    QDateTime now = QDateTime::currentDateTime();
    const QList<QByteArray>& timezoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray& tzId: timezoneIds) {
        QTimeZone timezone(tzId);


        int minutesOffset = timezone.offsetFromUtc(now) / 60;

        QString sign = "-";
        if (minutesOffset >= 0) {
            sign = "+";
        } else {
            minutesOffset *= -1;
        }

        QString hourStr = QString::number(int(minutesOffset / 60));
        if (hourStr.size() < 2) hourStr.prepend('0');

        QString minStr = QString::number(minutesOffset % 60);
        if (minStr.size() < 2) minStr.prepend('0');

        QVariantMap timezoneMap;
        timezoneMap["name"] = timezone.id();
        timezoneMap["city"] = timezone.id().sliced(timezone.id().indexOf('/') + 1);
        timezoneMap["offset"] = "UTC" + sign + hourStr + ":" + minStr;

        timezones.push_back(timezoneMap);
    }

    return timezones;
}

void DateTimeManagerCPP::callProcessFinished(const QJSValueList& args)
{
    if (mProcessFinishCb.isCallable()) {
        QJSValue result = mProcessFinishCb.call(args);

        if (result.isError()) {
            qDebug("%s:%s: %s",
                   qPrintable(result.property("fileName").toString()),
                   qPrintable(result.property("lineNumber").toString()),
                   qPrintable(result.toString().toStdString().c_str()));
        }

        //! Make it null.
        mProcessFinishCb = QJSValue(QJSValue::NullValue);
    }
}
