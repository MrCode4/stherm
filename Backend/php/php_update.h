#ifndef PHP_UPDATE_H
#define PHP_UPDATE_H

#include <QObject>
#include <QQmlEngine>

class php_update : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_update(QObject *parent = nullptr);

signals:

};

#endif // PHP_UPDATE_H
