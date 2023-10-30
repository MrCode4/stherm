#ifndef PHP_SYSTEM_H
#define PHP_SYSTEM_H

#include <QObject>
#include <QQmlEngine>

class php_system : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_system(QObject *parent = nullptr);

signals:

};

#endif // PHP_SYSTEM_H
