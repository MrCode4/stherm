#include "php_hardware.h"

// php_hardware::php_hardware(QObject *parent)
//     : QObject{parent}
// {

// }


php_hardware::php_hardware(DeviceConfig &config, QObject *parent)
    : deviceConfig(config), QObject{parent}
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
    // no code is added here, as the instantiation of the
}


