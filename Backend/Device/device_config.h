#pragma once

#include "nuve_types.h"

#define API_SERVER_BASE_URL "https://devapi.nuvehvac.com/"

namespace NUVE {

struct DeviceConfig
{
    DeviceConfig();
    DeviceConfig(const cpuid_t cpuid);

    void initialise(const cpuid_t cpuid);
    void init();
    void load();
    void save();
    // should be called only on constructor as env can not be changed during runtime
    void setEnv();

    //! Set the sample rate (maybe from server)
    void setSampleRate(const uint32_t &sr);

    // sampleRate use to save sensor data (minutes per record)
    uint32_t  sampleRate;

    uint32_t soft_v;
    uint32_t hard_v;
    uint32_t mode;
    uint32_t brightness;
    uint32_t brightness_mode;
    cpuid_t uid;
    std::string serial_number;
    std::string timezone;
    std::string technical_access_link;
    uint32_t backlight_type;
    bool backlight_status;
    uint32_t current_speed;
    std::string user_guide;
    bool start_pairing;
    bool wiring_check;
    bool is_service_titan;
    std::string timezone_number;
    bool qa_test;
    bool forget_sensor;
    uint32_t ventilator;
    uint32_t start_mode;
    bool shut_down;
    rgbVal_t scheme_backlight_rgb;
    uint32_t humidifier_id;
    std::string hum_wiring;
    uint32_t system_type;

    // endpoint for access API and internet access check
    std::string endpoint;

    // Configs to send test results to the specified cofigs
    std::string testConfigIp;
    std::string testConfigUser;
    std::string testConfigPassword;
    std::string testConfigDestination;

    // Enable/disable alert feature
    bool controlAlertEnabled;
};

} // namespace NUVE
