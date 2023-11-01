#include "php_hardware.h"


php_hardware::php_hardware(QObject *parent)
    : QObject{parent}
{

}




/**
 * @brief initialise the device to default values
 * This fucntion is only called if the device_config is not populated, so this will only be done on startup
 * 
 * TODO rename this to refelct its only done on intialisation or if it hasn't been done before...
 * 
 * @param uid its UID
 */
void php_hardware::setDefaultValues(uid_t uid)
{
    // TODO we need to pull these values from file (version.ini) and extract SOFTWARE_VERSION and HARDWARE_VERSION
    uint32_t swVer = 1;
    uint32_t hwVer = 1;
    // TODO implement this when current_stage is defined
    // DELETE FROM current_stage WHERE 1=1; INSERT INTO current_stage(mode,stage,timestamp,blink_mode,s2offtime) VALUES(0,0,current_timestamp,0,current_timestamp - interval '5 minute')", true);
    device_config.uid = uid;

}


