#include "DateTimeManager.h"

#include <QCoreApplication>
#include <QSettings>
#include <QTimeZone>

#include "LogHelper.h"
#include "NetworkInterface.h"
#include "RestApiExecutor.h"

Q_LOGGING_CATEGORY(DateTimeManagerLog, "DateTimeManager")
#define DTM_LOG TRACE_CATEGORY(DateTimeManagerLog)

// API Keys
const QString TIME_API_URL         = QString("https://worldtimeapi.org/api/timezone/Etc/UTC");
const QString UTC_TIMESTAMP_KEY    = QString("utc_datetime");

// Settings Keys
const QString UTC_DIFFERENCE_TIME_SETTINGS = QString("utcDifferenceTime");

DateTimeManager* DateTimeManager::mMe = nullptr;

DateTimeManager* DateTimeManager::me()
{
    if (!mMe) mMe = new DateTimeManager(qApp);

    return mMe;
}

DateTimeManager::DateTimeManager(QObject *parent)
    : QObject{parent}
    , mAutoUpdateTime { true }
    , mCurrentTimeZone { QTimeZone::systemTimeZone() }
    , mNow { QDateTime::currentDateTime() }
    , mEffectDst { true }
    , mRetryToGetCurrentTimeTimer { nullptr }
    , mNeedToSaveTimeDifference { false }
    , mNeedToCorrectTimeBaseLatest { false }
{
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);

    mProcess.setReadChannel(QProcess::StandardOutput);

    QSettings settings;
    if (settings.value("ntpSetAsDefaultAtFirstRun", false).toBool()) {
        setAutoUpdateTime(true);
        settings.setValue("ntpSetAsDefaultAtFirstRun", true);
    } else {
        //! Check auto-update time
        checkAutoUpdateTime();
    }

    //! Set up current date time timer
    mNowTimer.setInterval(100);
    mNowTimer.start();
    connect(&mNowTimer, &QTimer::timeout, this, [&]() {
        //! Update now
        mNow = QDateTime::currentDateTime();
        emit nowChanged();
    });

    connect(NetworkInterface::me(), &NetworkInterface::hasInternetChanged, this, [this]() {
        if (NetworkInterface::me()->hasInternet() && mAutoUpdateTime == false) {
            if (mNeedToSaveTimeDifference || mNeedToCorrectTimeBaseLatest) {
                getCurrentTimeFromServerAsync();
            }

        } else {
            stopGettingCurrentTime();
        }
    });
}

bool DateTimeManager::isRunning() const
{
    return (mProcess.state() == QProcess::Starting || mProcess.state() == QProcess::Running);
}

void DateTimeManager::setAutoUpdateTime(bool autoUpdate)
{
    if (isRunning() || mAutoUpdateTime == autoUpdate) {
        return;
    }

    connect(&mProcess, &QProcess::finished, this,
        [this, autoUpdate](int exitCode) {
            if (exitCode == 0) {
                setAutoUpdateTimeProperty(autoUpdate);
            }

            //! Call onfinished callback
            callProcessFinished({ exitCode });
        }, Qt::SingleShotConnection);

    if (autoUpdate) {
        enableTimeSyncdService();
        mProcess.start("systemctl", { "start" , "systemd-timesyncd" });
    } else {
        mProcess.start(TDC_COMMAND, { TDC_SET_NTP, "false" });
    }
}

bool DateTimeManager::autoUpdateTime() const
{
    return mAutoUpdateTime;
}

QVariant DateTimeManager::currentTimeZone() const
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

void DateTimeManager::setCurrentTimeZone(const QVariant& timezoneId)
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

    //! If new timezone has DST and user disabled DST use map to get equivalent non-DST timezone
    if (newCurrentTimezone.hasDaylightTime() && mTzMap.map.contains(timezoneId.toString())) {
        if (!mEffectDst) {
            setTimezoneTo(QTimeZone(mTzMap.map[timezoneId.toString()].toUtf8()));
            return;
        }
    }
    setTimezoneTo(newCurrentTimezone);
}

bool DateTimeManager::effectDst() const
{
    return mEffectDst;
}

void DateTimeManager::setEffectDst(bool newEffectDst)
{
    if (mEffectDst == newEffectDst) {
        return;
    }

    // to wait for when this is called right after timezone from server
    mProcess.waitForFinished();

    //! Also change timezone if needed
    //! If DST is true, change timezone to the original one not considering DST
    if (newEffectDst) {
        if (mCurrentTimeZone.id() != QTimeZone::systemTimeZoneId()) {
            setTimezoneTo(mCurrentTimeZone);
        }
    } else {
        //! If mCurrentTimeZone supports DST and user disabled it, set system timezone to the
        //! equivalent one.
        if (hasDST()) {
            QString nonDstTzId = mTzMap.map[mCurrentTimeZone.id()];
            if (QTimeZone nonDstTz = QTimeZone(nonDstTzId.toUtf8());
                nonDstTz.isValid() && nonDstTzId != QTimeZone::systemTimeZoneId()) {
                setTimezoneTo(nonDstTz);
            }
        }
    }

    mEffectDst = newEffectDst;
    emit effectDstChanged();
}

bool DateTimeManager::hasDST() const
{
    return mCurrentTimeZone.hasDaylightTime() && mTzMap.map.contains(mCurrentTimeZone.id());
}

QDateTime DateTimeManager::now() const
{
    return mNow;
}

void DateTimeManager::setDateTime(const QDateTime& datetime, bool calledFromUI)
{
    if (isRunning() || QDateTime::currentDateTime() == datetime) {
        return;
    }

    connect(&mProcess, &QProcess::finished, this, [this, calledFromUI](int exitCode, QProcess::ExitStatus) {
            //! Call onfinished callback
            callProcessFinished({ exitCode });

            if (calledFromUI && mAutoUpdateTime == false) {
                updateTimeDiffrenceBasedonServer();
            }
        }, Qt::SingleShotConnection);

    QString newDateTime = datetime.toString("yyyy-MM-dd hh:mm:ss");
    mProcess.start(TDC_COMMAND, {
                                    TDC_SET_TIME,
                                    newDateTime,
                                });
    // Device DateTime request sent by user or server.
    stopTimeCorrectionFromLatest();
}

QVariantList DateTimeManager::timezones() const
{
    QVariantList timezones;

    QDateTime now = QDateTime::currentDateTime();
    const QList<QByteArray>& timezoneIds = QTimeZone::availableTimeZoneIds();
    for (const QByteArray& tzId: timezoneIds) {
        QTimeZone timezone(tzId);

        if (timezone.isValid() && timezone.territory() != QLocale::AnyTerritory) {
            int minutesOffset = timezone.standardTimeOffset(now) / 60;

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
            timezoneMap["id"] = QString(timezone.id());
            timezoneMap["city"] = timezone.id().sliced(timezone.id().indexOf('/') + 1);
            timezoneMap["offset"] = "UTC" + sign + hourStr + ":" + minStr;

            timezones.push_back(timezoneMap);
        }
    }

    //! Sort timezones based on their offset
    std::sort(timezones.begin(), timezones.end(),
              [](const QVariant& first, const QVariant& second) {
        const QString firstOffset = first.toMap()["offset"].toString();
        const QString secondOffset = second.toMap()["offset"].toString();

        if (firstOffset.at(3) == '+' && secondOffset.at(3) == '-') {
            return false;
        } else if (firstOffset.at(3) == '-' && secondOffset.at(3) == '+') {
            return true;
        } else {
            const int fstHour = firstOffset.sliced(3, 3).toInt();
            const int fstMin = firstOffset.sliced(7).toInt();
            const int fstOffset = fstHour * 60 + fstMin;

            const int secHour = secondOffset.sliced(3, 3).toInt();
            const int secMin = secondOffset.sliced(7).toInt();
            const int secOffset = secHour * 60 + secMin;

            return fstOffset <= secOffset ? true : false;
        }
    });

    return timezones;
}

void DateTimeManager::checkAutoUpdateTime()
{
    mProcess.start(TDC_COMMAND, {
                                    TDC_SHOW,
                                    TDC_NTP_PROPERTY,
                                });
    mProcess.waitForFinished(300);

    if (mProcess.exitStatus() == QProcess::NormalExit && mProcess.exitCode() == 0) {
        auto line = mProcess.readLine();
        if (!line.isEmpty()){
            setAutoUpdateTimeProperty(line == "NTP=yes\n");
            return;
        }
    }

    qWarning() << "Error: DTM checkAutoUpdateTime failed to read output";
}

QString DateTimeManager::utcDateTimeToLocalString(const QString& utcDateTime,
                                                     const QString& inputFormat,
                                                     const QString& outputFormat)
{
    QDateTime dateTimeObject = QDateTime::fromString(utcDateTime, inputFormat);

    // Set time zone to UTC
    dateTimeObject.setTimeZone(QTimeZone::utc());

    // Convert to local time
    dateTimeObject = dateTimeObject.toTimeZone(mCurrentTimeZone);

    QString output = dateTimeObject.toString(outputFormat);
    if (output.isEmpty()) {
        DTM_LOG << "Conversion error: Date time" << utcDateTime
                            << "inputFormat: "  << inputFormat
                            << "outputFormat: " << outputFormat;
    }

    return output;
}

QString DateTimeManager::nowUTC(const QString& outputFormat)
{
    return QDateTime::currentDateTimeUtc().toString(outputFormat);
}

void DateTimeManager::enableTimeSyncdService()
{
    mServiceProcess.start("systemctl", {"enable", "systemd-timesyncd"});

    connect(&mServiceProcess,
            &QProcess::finished,
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    qWarning() << "Error: DTM enableTimeSyncdService failed to enable service";
                }
            });
}
void DateTimeManager::callProcessFinished(const QJSValueList& args)
{
    emit systemUpdated();

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

void DateTimeManager::setTimezoneTo(const QTimeZone& timezone)
{
    //! Set system time zone
    mProcess.start(TDC_COMMAND, {
                                    TDC_SET_TIMEZONE,
                                    timezone.id(),
                                });
}

void DateTimeManager::getCurrentTimeFromServerAsync() {

    stopGettingCurrentTime();

    if (!NetworkInterface::me()->hasInternet())
        return;

    auto callback = [this](const QDateTime &serverDateTimeUTC) {
        QSettings settings;
        QDateTime serverDateTimeUTCLocalTime = serverDateTimeUTC.toLocalTime();
        DTM_LOG << " server date time: " << serverDateTimeUTC << serverDateTimeUTCLocalTime;

        // When difference time does not exist, we cannot update the device`s DateTime, becouse it will be corrupted after apdate
        if (mNeedToCorrectTimeBaseLatest && settings.contains(UTC_DIFFERENCE_TIME_SETTINGS)) {

            qint64 secondsToAdd = settings.value(UTC_DIFFERENCE_TIME_SETTINGS, 0).toInt();

            QDateTime newDateTime = serverDateTimeUTCLocalTime.addSecs(secondsToAdd);
            setDateTime(newDateTime, false);
            mNeedToCorrectTimeBaseLatest = false;

            DTM_LOG << " secondsToAdd: " << secondsToAdd << " - newDateTime: " << newDateTime;
        }

        // If there is no time difference, we cannot update the device's DateTime,
        // as it may become corrupted after the update. We assume that the device's DateTime
        // is correct post-update, so we need to add UTC_DIFFERENCE_TIME_SETTINGS to the device settings.
        if (!settings.contains(UTC_DIFFERENCE_TIME_SETTINGS))
            mNeedToSaveTimeDifference = true;

        if (mNeedToSaveTimeDifference) {
            QDateTime datetime = QDateTime::currentDateTime();
            qint64 diffInSeconds = serverDateTimeUTCLocalTime.secsTo(datetime);

            settings.setValue(UTC_DIFFERENCE_TIME_SETTINGS, diffInSeconds);

            mNeedToSaveTimeDifference = false;
            DTM_LOG << " Saved diffrence time in seconds: " << diffInSeconds;
        }
    };

    auto errorCallback = [this, callback]() { scheduleRetryGetAsync(callback, 0); };

    getCurrentTimeOnlineAsync(callback, errorCallback);
}

void DateTimeManager::updateTimeDiffrenceBasedonServer()
{
    if (mNeedToSaveTimeDifference)
        return;

    mNeedToSaveTimeDifference = true;
    getCurrentTimeFromServerAsync();
}

void DateTimeManager::correctTimeBaseLatestState()
{
    if (mNeedToCorrectTimeBaseLatest)
        return;

    mNeedToCorrectTimeBaseLatest = true;
    getCurrentTimeFromServerAsync();
}

void DateTimeManager::stopTimeCorrectionFromLatest() {
    mNeedToCorrectTimeBaseLatest = false;
    if (!mNeedToSaveTimeDifference)
        stopGettingCurrentTime();
}

void DateTimeManager::getCurrentTimeOnlineAsync(std::function<void(const QDateTime &)> onSuccess,
                                                std::function<void()> onError)
{
    if (!NetworkInterface::me()->hasInternet())
        return;

    auto callback = [onSuccess,
                     onError](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[DateTimeManager] Network error while fetching time: "
                       << reply->errorString();
            if (onError) {
                onError();
            }

        } else  {
            QDateTime utcDateTime = QDateTime::fromString(data.value(UTC_TIMESTAMP_KEY).toString(),
                                                          Qt::ISODate);
            if (!utcDateTime.isValid()) {
                qWarning() << "[DateTimeManager] Invalid datetime format in API response";
                if (onError) {
                    onError();
                }

            } else if (onSuccess) {
                onSuccess(utcDateTime);

            } else {
                qWarning() << "[DateTimeManager] Invalid date time and invalid onSuccess";
            }
        }
    };

    RestApiExecutor *api = new RestApiExecutor(this);
    auto netReply = api->callGetApi(TIME_API_URL, callback, false);

    if (netReply)
        connect(netReply, &QNetworkReply::finished, api, &QObject::deleteLater);
    else
        api->deleteLater();
}

void DateTimeManager::scheduleRetryGetAsync(std::function<void(const QDateTime &)> callback,
                                            int retryCount)
{
    if (mNeedToSaveTimeDifference == false && mNeedToCorrectTimeBaseLatest == false) {
        return;
    }

    const int maxRetry = 5;
    if (retryCount >= maxRetry) {
        qWarning() << "[DateTimeManager] Max retries reached. retried due to " << mNeedToSaveTimeDifference << mNeedToCorrectTimeBaseLatest;
        return;
    }

    if (mRetryToGetCurrentTimeTimer == nullptr) {
        mRetryToGetCurrentTimeTimer = new QTimer(this);
        mRetryToGetCurrentTimeTimer->setSingleShot(true);

    } else {
        mRetryToGetCurrentTimeTimer->stop();
        mRetryToGetCurrentTimeTimer->disconnect();
    }

    int delayMs = calculateDelayTime(retryCount);

    DTM_LOG << " Retrying in" << delayMs / 1000 << "seconds (attempt"
            << retryCount + 1 << "of" << maxRetry << ")";

    connect(mRetryToGetCurrentTimeTimer, &QTimer::timeout, this, [this, callback, retryCount]() {
        getCurrentTimeOnlineAsync(callback, [this, callback, retryCount]() {
                scheduleRetryGetAsync(callback, retryCount + 1);
        });
    });

    mRetryToGetCurrentTimeTimer->start(delayMs);
}

void DateTimeManager::stopGettingCurrentTime()
{
    if (mRetryToGetCurrentTimeTimer != nullptr) {
        mRetryToGetCurrentTimeTimer->stop();
        delete mRetryToGetCurrentTimeTimer;
        mRetryToGetCurrentTimeTimer = nullptr;
    }
}

int DateTimeManager::calculateDelayTime(int retryCount)
{
    const int initialDelay = 1000; // 1 second in milliseconds
    const int factor = 3;          // Factor to increase time
    const int maxDelay = 180000;   // Max 180s

    int delay = initialDelay * std::pow(factor, retryCount);

    return qMin(delay, maxDelay);
}

QDateTime DateTimeManager::getCurrentTimeOnlineSync()
{

    // Default to system UTC time if network fails
    QDateTime time = QDateTime::currentDateTimeUtc();

    QEventLoop loop;
    auto callback = [&loop,
                     &time](QNetworkReply *reply, const QByteArray &rawData, QJsonObject &data) {
        QDateTime dateTime = QDateTime::fromString(data.value(UTC_TIMESTAMP_KEY).toString(),
                                                   Qt::ISODate);

        if (dateTime.isValid()) {
            time = dateTime;
        } else {
            qWarning() << "Invalid datetime format in API response (Sync)";
        }

        DTM_LOG << "getCurrentTime: " << data.value(UTC_TIMESTAMP_KEY).toString() << dateTime;
        loop.quit();
    };

    RestApiExecutor *api = new RestApiExecutor(this);
    QNetworkReply *netReply = api->callGetApi(TIME_API_URL,
                                              callback,
                                              false);

    if (netReply) {
        connect(netReply, &QNetworkReply::finished, api, &QObject::deleteLater);
        loop.exec();

    } else {
        api->deleteLater();
    }

    return time;
}

QString DateTimeManager::getCurrentTimeOnlineSyncAsString()
{
    return getCurrentTimeOnlineSync().toString(Qt::ISODate);
}

void DateTimeManager::setAutoUpdateTimeProperty(bool autoUpdate)
{
    if (!autoUpdate) {
        mNeedToSaveTimeDifference = false;
        mNeedToCorrectTimeBaseLatest = false;
        stopGettingCurrentTime();
    }

    if (mAutoUpdateTime != autoUpdate) {
        mAutoUpdateTime = autoUpdate;
        emit autoUpdateTimeChanged();
    }
}

bool operator!=(const QJSValue& left, const QJSValue& right)
{
    return !left.equals(right);
}
