#include "DateTimeManagerCPP.h"

#include <QTimeZone>

DateTimeManagerCPP::DateTimeManagerCPP(QObject *parent)
    : QObject{parent}
    , mAutoUpdateTime { false }
{
    //! Check auto-update time
    checkAutoUpdateTime();
}

bool DateTimeManagerCPP::isRunning() const
{
    return (mProcess.state() == QProcess::Starting || mProcess.state() == QProcess::Running);
}

void DateTimeManagerCPP::setAutoUpdateTime(bool autoUpdate)
{
    if (isRunning() || mAutoUpdateTime == autoUpdate) {
        return;
    }

    connect(&mProcess, &QProcess::finished, this,
        [this, autoUpdate](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                mAutoUpdateTime = autoUpdate;
                autoUpdateTimeChanged();
            }

            //! Call onfinished callback
            callProcessFinished({ exitCode });
        }, Qt::SingleShotConnection);

    mProcess.start(TDC_COMMAND, { TDC_SET_NTP, autoUpdate ? "true" : "false" });
}

bool DateTimeManagerCPP::autoUpdateTime() const
{
    return mAutoUpdateTime;
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

void DateTimeManagerCPP::checkAutoUpdateTime()
{
    mProcess.setReadChannel(QProcess::StandardOutput);
    mProcess.start(TDC_COMMAND, {
                                    TDC_SHOW,
                                    TDC_NTP_PROPERTY,
                                });
    mProcess.waitForFinished(40);

    if (mProcess.exitCode() == 0) {
        mAutoUpdateTime = (mProcess.readLine() == "NPT=yes\n");
    } else {
        qDebug() << Q_FUNC_INFO << __LINE__ << mProcess.readLine();
    }
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

bool operator!=(const QJSValue& left, const QJSValue& right)
{
    return !left.equals(right);
}
