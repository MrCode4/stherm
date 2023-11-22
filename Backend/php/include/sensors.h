#ifndef SENSORS_H
#define SENSORS_H

#include <QObject>

class Sensors : public QObject
{
    Q_OBJECT
public:
    uint32_t    sensor_id = 0;
    uint32_t    location_id;
    uint32_t    type_id;
    bool        is_main;
    std::string name;
    std::string sensor;
    bool        is_paired;
    bool        is_sent = false;
    bool        is_remove = false;


    explicit Sensors(QObject *parent = nullptr);

    void setDefaultValues(std::string sensor_name);

signals:

};

#endif // SENSORS_H
