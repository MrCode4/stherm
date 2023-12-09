#include "DateTimeManagerCPP.h"

#include <QTimeZone>

DateTimeManagerCPP::DateTimeManagerCPP(QObject *parent)
    : QObject{parent}
    , mAutoUpdateTime { false }
    , mCurrentTimeZone { QTimeZone::systemTimeZone() }
    , mNow { QDateTime::currentDateTime() }
{
    //! Check auto-update time
    checkAutoUpdateTime();

    //! Set up current date time timer
    mNowTimer.setInterval(100);
    mNowTimer.start();
    connect(&mNowTimer, &QTimer::timeout, this, [&]() {
        //! Update now
        mNow = QDateTime::currentDateTime();

        if (hasDST()) {
            mNow = mNow.addSecs(mCurrentTimeZone.daylightTimeOffset(mNow));
        }

        emit nowChanged();
    });
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

QVariant DateTimeManagerCPP::currentTimeZone() const
{
    const QString tzId = mCurrentTimeZone.id();
    int minutesOffset = mCurrentTimeZone.offsetFromUtc(QDateTime::currentDateTime()) / 60;

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

    return QVariantMap({
                        { "id", tzId },
                        { "city", tzId.sliced(tzId.indexOf('/') + 1) },
                        { "offset", QString("UTC") + sign + hourStr + ":" + minStr },
                        });
}

void DateTimeManagerCPP::setCurrentTimeZone(const QVariant& timezoneId)
{
    QTimeZone newCurrentTimezone(timezoneId.toByteArray());

    if (isRunning() || !newCurrentTimezone.isValid() || mCurrentTimeZone == newCurrentTimezone) {
        return;
    }

    connect(&mProcess, &QProcess::finished, this,
        [this, newCurrentTimezone](int exitCode, QProcess::ExitStatus) {
            if (exitCode == 0) {
                mCurrentTimeZone = newCurrentTimezone;
                emit currentTimeZoneChanged();
            }

            //! Call onfinished callback
            callProcessFinished({ exitCode });
        }, Qt::SingleShotConnection);

    //! Set system time zone
    mProcess.start(TDC_COMMAND, {
                                    TDC_SET_TIMEZONE,
                                    newCurrentTimezone.id(),
                                });

}

bool DateTimeManagerCPP::hasDST() const
{
    return mCurrentTimeZone.hasDaylightTime();
}

QDateTime DateTimeManagerCPP::now() const
{
    return mNow;
}

void DateTimeManagerCPP::setTime(const QDateTime& time)
{
    if (isRunning() || QDateTime::currentDateTime().time() == time.time()) {
        return;
    }

    connect(&mProcess, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus) {
            //! Call onfinished callback
            callProcessFinished({ exitCode });
        }, Qt::SingleShotConnection);

    mProcess.start(TDC_COMMAND, {
                                    TDC_SET_TIME,
                                    time.time().toString("hh:mm:ss"),
                                });
}

QVariantList DateTimeManagerCPP::timezones() const
{
    QVariantList timezones;

    QDateTime now = QDateTime::currentDateTime();
    const QList<QByteArray>& timezoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray& tzId: timezoneIds) {
        QTimeZone timezone(tzId);

        if (timezone.isValid() && timezone.territory() != QLocale::AnyTerritory) {
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
            timezoneMap["id"] = timezone.id();
            timezoneMap["city"] = timezone.id().sliced(timezone.id().indexOf('/') + 1);
            timezoneMap["offset"] = "UTC" + sign + hourStr + ":" + minStr;

            timezones.push_back(timezoneMap);
        }
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
        mAutoUpdateTime = (mProcess.readLine() == "NTP=yes\n");
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
