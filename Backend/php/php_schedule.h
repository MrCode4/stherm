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

signals:

};

#endif // PHP_SCHEDULE_H
