#include "device_config.h"

#include <QSettings>
#include <QtGlobal>

#include "LogHelper.h"

NUVE::DeviceConfig::DeviceConfig() {
    init();
    load();
    setEnv();
}

NUVE::DeviceConfig::DeviceConfig(const cpuid_t cpuid) {
    initialise(cpuid);
    setEnv();
}

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

    // Save a record every 1 minute
    // loaded at startup if exits in config! changing this in runtime.
    sampleRate = 1;

    soft_v = swVer;
    hard_v = hwVer;
    mode = 1;
    brightness = 100;
    brightness_mode = 0;
    serial_number = "";
    timezone = "Pacific/Midway";
    backlight_type = 0;
    backlight_status = false;
    current_speed = 0;
    user_guide = "";
    start_pairing = false;
    wiring_check = true;
    is_service_titan = false;
    timezone_number = "";
    qa_test = false;
    forget_sensor = false;
    ventilator = 0;
    start_mode = 0;
    shut_down = false;
    scheme_backlight_rgb = {0, 0, 0};
    humidifier_id = 3;
    hum_wiring = "";
    system_type = 1;
    endpoint = API_SERVER_BASE_URL;
    controlAlertEnabled = false;
}

void NUVE::DeviceConfig::setSampleRate(const uint32_t& sr) {
    if (sampleRate == sr)
        return;

    sampleRate = sr;
    save();
}

void NUVE::DeviceConfig::load()
{
    QSettings config("/usr/local/bin/device_config.ini", QSettings::IniFormat);
    uid = config.value("uid").toString().toStdString();
    serial_number = config.value("serial_number").toString().toStdString();

    endpoint = config.value("endpoint", API_SERVER_BASE_URL).toString().toStdString();

    bool ok;
    auto sr = config.value("sampleRate").toInt(&ok);
    if (ok)
        sampleRate = sr;
}

void NUVE::DeviceConfig::save()
{
    QSettings config("/usr/local/bin/device_config.ini", QSettings::IniFormat);

    config.setValue("uid", QString::fromStdString(uid));
    config.setValue("serial_number", QString::fromStdString(serial_number));
    config.setValue("endpoint", QString::fromStdString(endpoint));
    config.setValue("sampleRate", QString::number(sampleRate));
}

void NUVE::DeviceConfig::setEnv()
{
    //! Set check internet access url in env, used by NmcliInterface
    qputenv("NMCLI_INTERNET_ACCESS_URL", QByteArray::fromStdString(endpoint));
    //! used for Sync class calling API
    qputenv("API_SERVER_BASE_URL", QByteArray::fromStdString(endpoint));
}
