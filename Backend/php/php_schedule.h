#ifndef PHP_SCHEDULE_H
#define PHP_SCHEDULE_H

#include <QObject>
#include <QQmlEngine>

class php_schedule : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_schedule(QObject *parent = nullptr);

    void getScheduleList(void);
    void enableSchedule(void);
    void removeSchedule(void);
    void checkScheduleName(void);

    void getSchedule(void);
    void setSchedule(void);


signals:

};

#endif // PHP_SCHEDULE_H
