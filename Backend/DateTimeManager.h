#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QProcess>
#include <QTimeZone>
#include <QTimer>

#include "ProcessExecutor.h"
#include "TimezonesDSTMap.h"

#define TDC_COMMAND         "timedatectl"
#define TDC_SHOW            "show"
#define TDC_SET_NTP         "set-ntp"
#define TDC_SET_TIME        "set-time"
#define TDC_SET_TIMEZONE    "set-timezone"
#define TDC_NTP_PROPERTY    "--property=NTP"

/*!
 * \brief The DateTimeManager class provides an interface to interact with system date and time
 * useing \a\b timedatectl.
 */
class DateTimeManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariant currentTimeZone READ currentTimeZone WRITE setCurrentTimeZone NOTIFY currentTimeZoneChanged FINAL)
    Q_PROPERTY(bool hasDST READ hasDST NOTIFY currentTimeZoneChanged)
    Q_PROPERTY(bool autoUpdateTime READ autoUpdateTime WRITE setAutoUpdateTime NOTIFY autoUpdateTimeChanged)
    Q_PROPERTY(QJSValue onfinish  MEMBER  mProcessFinishCb)
    Q_PROPERTY(QDateTime now READ now NOTIFY nowChanged)
    Q_PROPERTY(bool effectDst READ effectDst WRITE setEffectDst NOTIFY effectDstChanged FINAL)

private:
    explicit DateTimeManager(QObject *parent = nullptr);

public:
    static DateTimeManager* me();
    static DateTimeManager* create(QQmlEngine*, QJSEngine*) {return me();}

    /*!
     * \brief isRunning Checks if internal process is running
     * \return
     */
    bool            isRunning() const;

    /*!
     * \brief setAutoUpdateTime Set auto updat time off/on
     * \param autoUpdate
     */
    void            setAutoUpdateTime(bool autoUpdate);
    bool            autoUpdateTime() const;

    QVariant        currentTimeZone() const;
    void            setCurrentTimeZone(const QVariant& timezoneId);

    bool            effectDst() const;
    void            setEffectDst(bool newEffectDst);

    bool            hasDST() const;

    QDateTime       now() const;

    /*!
     * \brief setDateTime Set system datetime to given datetime.
     * \param datetime
     * \return
     */
    Q_INVOKABLE void            setDateTime(const QDateTime& datetime, bool calledFromUI = true);

    /*!
     * \brief timeZones Returns a list of all the timezones avaialable in the system.
     * \return A list of json containing timezone name and offset
     */
    Q_INVOKABLE QVariantList    timezones() const;

    /*!
     * \brief checkAutoUpdateTime Checks if auto update time is enabled using \a\b timedatectl
     */
    Q_INVOKABLE void            checkAutoUpdateTime();


    /*!
     * \brief utcDateTimeToLocalString Convert UTC Date time to local date time
     * \return QString of local date time with outputFormat format
     */
    Q_INVOKABLE QString         utcDateTimeToLocalString(const QString& utcDateTime,
                                                 const QString& inputFormat  = QString("yyyy-MM-dd HH:mm:ss"),
                                                 const QString& outputFormat = QString("MMM dd, yyyy h:mmAP"));

    /*!
     * \brief Now in UTC
     * \return QString of Now date time with outputFormat format
     */
    Q_INVOKABLE QString         nowUTC(const QString &outputFormat = QString("yyyy-MM-dd HH:mm:ss"));

    Q_INVOKABLE void enableTimeSyncdService();

    Q_INVOKABLE void correctTimeBaseLatestState();
    Q_INVOKABLE void stopTimeCorrectionFromLatest();

    Q_INVOKABLE void stopGettingCurrentTime();
    Q_INVOKABLE QString getCurrentTimeOnlineSyncAsString();

    Q_INVOKABLE void updateTimeDiffrenceBasedonServer();

private:
    /*!
     * \brief setAutoUpdateTimeProperty Sets mAutoUpdateTime value
     * \param autoUpdate
     */
    void setAutoUpdateTimeProperty(bool autoUpdate);

    /*!
     * \brief callProcessFinished Calls \ref onfinish callback if its a callable
     * \param args
     */
    void callProcessFinished(const QJSValueList& args);

    /*!
     * \brief setTimezoneTo Sets system timezone to the given
     * \param timezone
     */
    void setTimezoneTo(const QTimeZone& timezone);

    QDateTime getCurrentTimeOnlineSync();
    bool getCurrentTimeOnlineAsync(std::function<void(const QDateTime &)> onSuccess,
                                   std::function<void()> onError);

    void scheduleRetryGetAsync(std::function<void(const QDateTime &)> callback, int retryCount);

    int calculateDelayTime(int retryCount);

    void getCurrentTimeFromServerAsync();

signals:
    void autoUpdateTimeChanged();
    void currentTimeZoneChanged();
    void nowChanged();
    void effectDstChanged();
    void systemUpdated();

private:
    static DateTimeManager* mMe;

    //!
    //! \brief mProcess The process that is used to call timedatectl
    //!
    mutable QProcess    mProcess;

    //!
    //! \brief mServiceProcess The process that is used to call systemctl
    //!
    mutable QProcess mServiceProcess;

    //!
    //! \brief mAutoUpdateTime
    //!
    bool                mAutoUpdateTime;

    //!
    //! \brief mProcessFinishCb This js value is called when \ref mProcess is finished if its a
    //! callable
    //!
    QJSValue            mProcessFinishCb;

    //!
    //! \brief mCurrentTimeZone Holds system current timezone
    //!
    QTimeZone           mCurrentTimeZone;

    //!
    //! \brief mNow Holds current date time
    //!
    QDateTime           mNow;

    //!
    //! \brief mNowTimer This timer is used to update current date time
    //!
    QTimer              mNowTimer;

    //!
    //! \brief mEffectDst Whether DST should effect or not.
    //!
    bool                mEffectDst;

    //!
    //! \brief mTzMap
    //!
    TimezonesDSTMap     mTzMap;

    QTimer *mRetryToGetCurrentTimeTimer;

    bool mNeedToSaveTimeDifference;
    bool mNeedToCorrectTimeBaseLatest;

    QSharedPointer<ProcessExecutor> mProcessExecutor;
};

/*!
 * \brief operator!= This operator is required to be able to use \a\b QJSValue inside a
 * \b Q_PROPERTY as a MEMBER
 * \param left
 * \param right
 * \return
 */
bool operator!=(const QJSValue& left, const QJSValue& right);

