#ifndef DATETIMEMANAGERCPP_H
#define DATETIMEMANAGERCPP_H

#include <QObject>
#include <QQmlEngine>
#include <QProcess>

/*!
 * \brief The DateTimeManagerCPP class provides an interface to interact with system date and time
 * useing \a\b timedatectl.
 */
class DateTimeManagerCPP : public QObject
{
    Q_OBJECT

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
    Q_INVOKABLE void            setAutoUpdateTime(bool autoUpdate);

    /*!
     * \brief timeZones Returns a list of all the timezones avaialable in the system.
     * \return A list of json containing timezone name and offset
     */
    Q_INVOKABLE QVariantList    timezones() const;

private:
    /*!
     * \brief callProcessFinished Calls \ref onfinish callback if its a callable
     * \param args
     */
    void callProcessFinished(const QJSValueList& args);

private:
    //!
    //! \brief mProcess The process that is used to call timedatectl
    //!
    QProcess    mProcess;

    //!
    //! \brief mProcessFinishCb This js value is called when \ref mProcess is finished if its a
    //! callable
    //!
    QJSValue    mProcessFinishCb;
};

#endif // DATETIMEMANAGERCPP_H
