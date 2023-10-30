#ifndef PHP_SENSORS_H
#define PHP_SENSORS_H

#include <QObject>
#include <QQmlEngine>

class php_sensors : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_sensors(QObject *parent = nullptr);

signals:

};

#endif // PHP_SENSORS_H
