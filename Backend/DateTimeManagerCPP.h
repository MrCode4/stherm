#ifndef DATETIMEMANAGERCPP_H
#define DATETIMEMANAGERCPP_H

#include <QObject>
#include <QQmlEngine>

/*!
 * \brief The DateTimeManagerCPP class provides an interface to interact with system date and time
 */
class DateTimeManagerCPP : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit DateTimeManagerCPP(QObject *parent = nullptr);

    /*!
     * \brief timeZones Returns a list of all the timezones avaialable in the system.
     * \return A list of json containing timezone name and offset
     */
    Q_INVOKABLE QVariantList timezones() const;
};

#endif // DATETIMEMANAGERCPP_H
