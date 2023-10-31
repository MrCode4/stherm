#ifndef PHP_SENSORS_H
#define PHP_SENSORS_H

#include <QObject>
#include <QQmlEngine>

class php_sensors : public QObject
{
    Q_OBJECT
    //    QML_ELEMENT
public:
    explicit php_sensors(QObject *parent = nullptr);

    void getSensorsData(void);
    void getSensorList(void);
    void remove(void);
    void getSensorPairList(void);
    void setSensor(void);
    void checkSensorName(void);
    void getSensorInfo(void);
    void getSensorLocations(void);

    void startEndPairing(void);

signals:

};

#endif // PHP_SENSORS_H
