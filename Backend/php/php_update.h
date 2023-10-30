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

    void getSoftwareUpdateInfo(void);
    void install(void);


signals:

};

#endif // PHP_UPDATE_H
