#ifndef DATETIMEMANAGERCPP_H
#define DATETIMEMANAGERCPP_H

#include <QObject>
#include <QQmlEngine>
#include <QProcess>

#define TDC_COMMAND         "timedatectl"
#define TDC_SHOW            "show"
#define TDC_SET_NTP         "set-ntp"
#define TDC_SET_TIME        "set-time"
#define TDC_NTP_PROPERTY    "--property=NTP"

/*!
 * \brief The DateTimeManagerCPP class provides an interface to interact with system date and time
 * useing \a\b timedatectl.
 */
class DateTimeManagerCPP : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool autoUpdateTime READ autoUpdateTime WRITE setAutoUpdateTime NOTIFY autoUpdateTimeChanged)
    Q_PROPERTY(QJSValue onfinish  MEMBER  mProcessFinishCb)

    QML_ELEMENT
public:
    explicit DateTimeManagerCPP(QObject *parent = nullptr);

    /*!
     * \brief isRunning Checks if internal process is running
     * \return
     */
    bool isRunning() const;

    /*!
     * \brief setAutoUpdateTime Set auto updat time off/on
     * \param autoUpdate
     */
    void            setAutoUpdateTime(bool autoUpdate);
    bool            autoUpdateTime() const;


    /*!
     * \brief setTime Set system time to given time.
     * \param time
     * \return
     */
    Q_INVOKABLE void            setTime(const QDateTime& time);

    /*!
     * \brief timeZones Returns a list of all the timezones avaialable in the system.
     * \return A list of json containing timezone name and offset
     */
    Q_INVOKABLE QVariantList    timezones() const;

private:
    /*!
     * \brief checkAutoUpdateTime Checks if auto update time is enabled using \a\b timedatectl
     */
    void checkAutoUpdateTime();

    /*!
     * \brief callProcessFinished Calls \ref onfinish callback if its a callable
     * \param args
     */
    void callProcessFinished(const QJSValueList& args);

signals:
    void autoUpdateTimeChanged();

private:
    //!
    //! \brief mProcess The process that is used to call timedatectl
    //!
    mutable QProcess    mProcess;

    //!
    //! \brief mAutoUpdateTime
    //!
    bool                mAutoUpdateTime;

    //!
    //! \brief mProcessFinishCb This js value is called when \ref mProcess is finished if its a
    //! callable
    //!
    QJSValue            mProcessFinishCb;
};

/*!
 * \brief operator!= This operator is required to be able to use \a\b QJSValue inside a
 * \b Q_PROPERTY as a MEMBER
 * \param left
 * \param right
 * \return
 */
bool operator!=(const QJSValue& left, const QJSValue& right);

#endif // DATETIMEMANAGERCPP_H
