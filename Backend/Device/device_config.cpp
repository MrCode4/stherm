#include "device_config.h"

#include <QSettings>

#include "LogHelper.h"

void NUVE::DeviceConfig::initialise(const cpuid_t cpuid)
{
    uid = cpuid;
    init();
    save();
}

void NUVE::DeviceConfig::init()
{
    // TODO we need to pull these values from config file (version.ini) and extract SOFTWARE_VERSION and HARDWARE_VERSION
    uint32_t swVer = 100;
    uint32_t hwVer = 1;
    // TODO implement this when current_stage is defined

    // DELETE FROM current_stage WHERE 1=1; INSERT INTO current_stage(mode,stage,timestamp,blink_mode,s2offtime) VALUES(0,0,current_timestamp,0,current_timestamp - interval '5 minute')", true);

    soft_v = swVer;
    hard_v = hwVer;
    mode = 1;
    brightness = 100;
    brightness_mode = 0;
    serial_number = "";
    timezone = "Pacific/Midway";
    technical_access_link = TECHNIC_QR;
    backlight_rgb = {0, 0, 0};
    backlight_type = 0;
    backlight_status = false;
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
    scheme_backlight_rgb = {0, 0, 0};
    humidifier_id = 3;
    hum_wiring = "";
    system_type = 1;
    emergency_heating = 0;
    ob_state = "cool";
    technical_edit_link = TECHNIC_EDIT_QR;
}

void NUVE::DeviceConfig::load()
{
    QSettings config("/usr/local/bin/device_config.ini", QSettings::IniFormat);
    uid = config.value("uid").toString().toStdString();
    serial_number = config.value("serial_number").toString().toStdString();
}

void NUVE::DeviceConfig::save()
{
    QSettings config("/usr/local/bin/device_config.ini", QSettings::IniFormat);

    config.setValue("uid", QString::fromStdString(uid));
    config.setValue("serial_number", QString::fromStdString(serial_number));
}
