#pragma once

#include "nuve_types.h"

#include <QElapsedTimer>
#include <QDateTime>

namespace NUVE {

struct Timing
{
    Timing() = default;

    QElapsedTimer       s1uptime;
    QElapsedTimer       uptime;
    QElapsedTimer       s2uptime;
    QElapsedTimer       s2Offtime;
    QElapsedTimer       fanTime;

    bool s2hold;
    bool s3hold;
    bool alerts;
    timestamp_t set_backlight_time;
    uint32_t wiring_check_interval;
    timestamp_t wiring_check_timestamp;
    uint32_t contractor_info_interval;
    timestamp_t contractor_info_timestamp;
    uint32_t info_update_interval;
    timestamp_t info_update_timestamp;
    timestamp_t soft_update_timestamp;

    QDateTime            fan_time;

    uint32_t start_fan_timing;
    timestamp_t delete_info_timestamp;
    std::string delete_info_interval;

    void setDefaultValues(void);

    void refreshTimestamps(void);
};

} // namespace NUVE
