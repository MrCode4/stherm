#include "sensors.h"

void NUVE::Sensors::setDefaultValues(std::string sensor_name)
{
    if (is_main) {
        sensor = sensor_name;
        is_sent = false;
    }
}
