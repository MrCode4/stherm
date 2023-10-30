#ifndef PHP_HARDWARE_H
#define PHP_HARDWARE_H

#include <QObject>
#include <QQmlEngine>

class php_hardware : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_hardware(QObject *parent = nullptr);

signals:

};

#endif // PHP_HARDWARE_H
