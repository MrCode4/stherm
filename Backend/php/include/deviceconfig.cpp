#include "deviceconfig.h"

DeviceConfig::DeviceConfig(const std::string &hex_uid, QObject *parent)
    : QObject{parent}
{
    // TODO we need to pull these values from config file (version.ini) and extract SOFTWARE_VERSION and HARDWARE_VERSION
    uint32_t swVer = 100;
    uint32_t hwVer = 1;
    // TODO implement this when current_stage is defined

    // DELETE FROM current_stage WHERE 1=1; INSERT INTO current_stage(mode,stage,timestamp,blink_mode,s2offtime) VALUES(0,0,current_timestamp,0,current_timestamp - interval '5 minute')", true);

    soft_v = swVer;
    hard_v = hwVer;
    mode = 1;
    brightness = 80;
    brightness_mode = 0;
    serial_number = "";
    uid = strtoll(hex_uid.c_str(), NULL, 16);
    timezone = "Pacific/Midway";
    technical_access_link = TECHNIC_QR;
    backlight_rgb = {0,0,0};
    backlight_type = 0;
    backlight_status = false;
    last_update = current_timestamp();
    server_last_update = current_timestamp();
    current_speed = 0;
    logo = "nexgen.png";
    phone = "";
    url = "";
    user_guide = "";
    start_pairing = false;
    wiring_check = true;
    is_service_titan = false;
    timezone_number = "";
    qa_test = false;
    forget_sensor = false;
    contractor_name = "NextGen";
    ventilator = 0;
    start_mode = 0;
    shut_down = false;
    scheme_backlight_rgb = {0,0,0};
    humidifier_id = 3;
    hum_wiring = "";
    system_type = 1;
    emergency_heating = 0;
    ob_state = "cool";
    technical_edit_link  = TECHNIC_EDIT_QR;

}
