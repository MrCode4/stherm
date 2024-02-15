#include "hardware.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "LogHelper.h"
#include "UtilityHelper.h"

#include "Device/current_stage.h"
#include "Device/device_config.h"
#include "Device/sensors.h"
#include "Device/timing.h"

#include "System.h"

NUVE::Hardware::Hardware(DeviceConfig &config,
                         Timing &tim,
                         CurrentStage &stage,
                         Sensors &sens,
                         System &sys,
                         QObject *parent)
    : QObject(parent)
    , deviceConfig(config)
    , timing(tim)
    , currentStage(stage)
    , sensors(sens)
    , system(sys)
{
    // TODO deviceConfig, timing and current stage should be loaded from NV
}

/**
 * @brief initialise the device to default values
 * This fucntion is only called if the device_config is not populated, so this will only be done on startup
 * 
 * TODO rename this to refelct its only done on intialisation or if it hasn't been done before...
 * 
 * @param uid its UID
 */
void NUVE::Hardware::setDefaultValues(cpuid_t uid)
{
    // no code is added here, as the instantiation of the
    deviceConfig.initialise(uid);
    timing.setDefaultValues();
    currentStage.setDefaultValues();
}

// TODO we recieve and propagate the uid here, but this is really just becuase I dont want to introduce the getter here
int NUVE::Hardware::runDevice(cpuid_t uid)
{
    if (deviceConfig.uid.empty()) {
        TRACE << "device config not initialised, maybe FIRST RUN.\n                                setting default values";
        setDefaultValues(uid);
    } else {
        // Configuration exists, so just update timing
        timing.refreshTimestamps();
        currentStage.timestamp = current_timestamp();
    }

    // TODO I'm not sure if there is just one sensor reading or multiple, we assume one but its most likely this is an array
    // TODO need to convert UID to text and put he UID in here....
    std::ostringstream oss;
    oss << std::hex << deviceConfig.uid;
    sensors.setDefaultValues(oss.str());

    // TODO the wifi code below will check if there is a backup ini file in update partition, then load and delete it.  This may not work in all cases for restore
    // TODO
    // Check if wifi.ini is saved in backup partition, if so, then read it, store in database, delete

    // TODO Clean the backup partition completely (rm /mnt/.../*)

    // TODO Check if each of these files exists and delete them:
    // /usr/share/apache2/default-site/htdocs/update.zip    -- partial update file?
    //  /usr/share/apache2/default-site/htdocs/update       -- partial update file?
    // need_recover.txt                                     -- ??

    //    set the time zone from config file

    return 0;
}

///** Send a CURL request to the remote server to look up the UID and return the
// *  serial number associated with it */
//bool NUVE::Hardware::getSN(cpuid_t uid, std::string &sn)
//{
//    return !sn.empty();
//}

int NUVE::Hardware::getStartMode()
{
    // then check the start mode of the device, to see if this is the first run
    // TODO the getstartmode exe would read export, then read GPIO90, and return 0 if high, and 1 if low
    // if == 0, set in device config, then return
    uint32_t sm = UtilityHelper::getStartMode();

    LOG_DEBUG(QString("start mode started : %0").arg(sm));

    if (sm < 0) {
        // TODO this is a critical error.... how to handle it
        qWarning() << "Start mode encounter an error!";
    }

    return sm;
}

void NUVE::Hardware::getWiringsFromServer(cpuid_t uid)
{
    // TODO first send the request (sync, getWirings)
    // TODO Then update the wirings variable
    //  note that here may be multiple
}
