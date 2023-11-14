#include <ctime>

#include "timing.h"

Timing::Timing(QObject *parent)
    : QObject{parent}
{
}

void Timing::setDefaultValues(void)
{
    int year = 2023 - 1900, month = 0, day = 1, hour = 0, minute = 0, second = 0;
    std::tm tm = {.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 2023-1900, .tm_isdst = LOCALE_USE_DST };
    timestamp_t tr = std::mktime(&tm);

    uptime = current_timestamp();
    s1uptime = current_timestamp();
    s2uptime = current_timestamp();
    s2hold = false;
    s3hold = false;
    alerts = false;
    set_backlight_time = current_timestamp();
    wiring_check_interval = 10;
    wiring_check_timestamp = current_timestamp();
    contractor_info_interval = 1;
    contractor_info_timestamp = tr;
    info_update_interval = 15;
    info_update_timestamp = current_timestamp();
    soft_update_timestamp = tr;
    fan_time = current_timestamp() - minuteToTimestamp(5);
    start_fan_timing = 0;
    delete_info_timestamp = current_timestamp();
    delete_info_interval = DELETE_INFO_INTERVAL;
}

void Timing::refreshTimestamps(void)
{
    int year = 2023 - 1900, month = 0, day = 1, hour = 0, minute = 0, second = 0;
    std::tm tm = {.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 2023-1900, .tm_isdst = LOCALE_USE_DST };
    timestamp_t tr = std::mktime(&tm);

    uptime = current_timestamp();
    s2uptime = current_timestamp();
    set_backlight_time = current_timestamp();
    wiring_check_timestamp = current_timestamp();
    contractor_info_timestamp = tr;
    info_update_timestamp = current_timestamp();
    soft_update_timestamp = tr;
    fan_time = current_timestamp() - minuteToTimestamp(5);
}
