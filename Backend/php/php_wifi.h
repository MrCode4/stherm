#ifndef PHP_WIFI_H
#define PHP_WIFI_H

#include <QObject>
#include <QQmlEngine>

class php_wifi : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_wifi(QObject *parent = nullptr);
    
    void getWifiList(void);
    void connect(void);
    void forgot(void);
    void disconnect(void);
    void manualConnect(void);

signals:

};

#endif // PHP_WIFI_H
