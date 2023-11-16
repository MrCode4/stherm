#pragma once

#include "nuve_types.h"

namespace NUVE {

struct Sensors
{
    Sensors() = default;

    uint32_t sensor_id = 0;
    uint32_t location_id;
    uint32_t type_id;
    bool is_main;
    std::string name;
    std::string sensor;
    bool is_paired;
    bool is_sent = false;
    bool is_remove = false;

    void setDefaultValues(std::string sensor_name);
};

} // namespace NUVE
