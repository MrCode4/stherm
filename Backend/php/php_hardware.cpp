#include "php_hardware.h"

// php_hardware::php_hardware(QObject *parent)
//     : QObject{parent}
// {

// }


php_hardware::php_hardware(DeviceConfig &config, Timing &tim, CurrentStage &stage, Sensors &sens, QObject *parent)
    : deviceConfig(config), timing(tim), currentStage(stage), sensors(sens), QObject{parent}
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
void php_hardware::setDefaultValues(cpuid_t uid)
{
    // no code is added here, as the instantiation of the
    deviceConfig.initialise(uid);
    timing.setDefaultValues();
    currentStage.setDefaultValues();
}

// TODO refactor this code actually peforms an initial configuration
int php_hardware::runDevice(cpuid_t uid)
{
    if (deviceConfig.uid != 0) {
//        qDebug << Q_FUNC_INFO << "device config not initialised";
        setDefaultValues(uid);
    }
    else {
        // Configuration exists, so just update timing
        timing.refreshTimestamps();
        currentStage.timestamp = current_timestamp();
    }
    // TODO I'm not sure if there is just one sensor reading or multiple, we assume one but its most likely this is an array
    // TODO need to convert UID to text and put he UID in here....
    sensors.setDefaultValues("UID TEST");


    return 0;
}

// TODO refactor, this does MUCH more than just get the start mode
// TODO we recieve and propagate the uid here, but this is really just becuase I dont want to introduce the getter here
int php_hardware::getStartMode(cpuid_t uid)
{
    // Start by calling runDevice, which will load and populate the device config
    runDevice(uid);
    // then check the start mode of the device, to see if this is the first run
    // TODO the getstartmode exe would read export, then read GPIO90, and return 0 if high, and 1 if low
    // if == 0, set in device config, then return
    uint32_t sm = UtilityHelper::getStartMode();
    if (sm < 0) {
        // TODO this is a critical error.... how to handle it
    }
    if (sm == 0) {
        deviceConfig.start_mode = 0;
        return 0;
    }
    deviceConfig.start_mode = 1;

    // Check if device is newly updated: call system->getIsDeviceUpdated()



}



