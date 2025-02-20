#include "sensors.h"

Sensors::Sensors(QObject *parent)
    : QObject{parent}
{

}

void Sensors::setDefaultValues(std::string sensor_name)
{
    if (is_main) {
        sensor = sensor_name;
        is_sent = false;
    }
}
