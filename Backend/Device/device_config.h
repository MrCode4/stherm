#pragma once

#include "nuve_types.h"

namespace NUVE {

struct DeviceConfig
{
    DeviceConfig() = default;
    DeviceConfig(const cpuid_t cpuid) { initialise(cpuid); }

    void initialise(const cpuid_t cpuid);

    timestamp_t last_update;
    timestamp_t server_last_update;
    uint32_t soft_v;
    uint32_t hard_v;
    uint32_t mode;
    uint32_t brightness;
    uint32_t brightness_mode;
    cpuid_t uid;
    std::string serial_number;
    std::string timezone;
    std::string technical_access_link;
    rgbVal_t backlight_rgb;
    uint32_t backlight_type;
    bool backlight_status;
    uint32_t current_speed;
    std::string logo;
    std::string phone;
    std::string url;
    std::string user_guide;
    bool start_pairing;
    bool wiring_check;
    bool is_service_titan;
    std::string timezone_number;
    bool qa_test;
    bool forget_sensor;
    std::string contractor_name;
    uint32_t ventilator;
    uint32_t start_mode;
    bool shut_down;
    rgbVal_t scheme_backlight_rgb;
    uint32_t humidifier_id;
    std::string hum_wiring;
    uint32_t system_type;
    uint32_t emergency_heating;
    std::string ob_state;
    std::string technical_edit_link;
};

} // namespace NUVE
