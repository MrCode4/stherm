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


    void getStartMode(void);

    void setBacklight(void);
    void getBacklight(void);

    void getWiring(void);
    
    void getActualWiring(void);

    void checkClient(void);

    void getSettings(void);
    void setSettings(void);

    void setBrightness(void);


signals:

};

#endif // PHP_HARDWARE_H
