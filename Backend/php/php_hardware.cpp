#include "php_hardware.h"
#include "LogHelper.h"

#include <Backend/Core/System.h>
// php_hardware::php_hardware(QObject *parent)
//     : QObject{parent}
// {

// }

php_hardware::php_hardware(
    DeviceConfig &config, Timing &tim, CurrentStage &stage, Sensors &sens, QObject *parent)
    : QObject(parent)
    , deviceConfig(config)
    , timing(tim)
    , currentStage(stage)
    , sensors(sens)
{
    m_system = new NUVE::System(nullptr, this);
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
    if (deviceConfig.uid.empty()) {
        //        qDebug << Q_FUNC_INFO << "device config not initialised";
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


    return 0;
}

/** Send a CURL request to the remote server to look up the UID and return the
 *  serial number associated with it */
bool php_hardware::getSN(cpuid_t uid, std::string &sn)
{
    // TODO this function should probably cache the serial number to avoid duplicated look ups?  QUESTION: should we retrieve once only for the device, once every boot, or only after update?

    // TODO curl send will give the serial nubmer and a bool representing if hte client_id is >0
    // TODO send the http request
    sn = m_system->getSN(uid);
    return true;
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

    LOG_DEBUG(QString("start mode started : %0").arg(sm));

    if (sm < 0) {
        // TODO this is a critical error.... how to handle it
    }
    if (sm == 0) {
        deviceConfig.start_mode = 0;
        return 0;
    }
    deviceConfig.start_mode = 1;

    // Check if device is newly updated: call system->getIsDeviceUpdated()
    if (true == UtilityHelper::tempIsUpdated())
    {
        getWiringsFromServer(uid);
        UtilityHelper::tempClearUpdatedFlag();
    }

    // Check serial number
    if (deviceConfig.serial_number != "")
    {
        // serial number already set, starting normally
// TODO what do these return values mean?
        return 1;
    }
    std::string sn;
    if (getSN(uid, sn)) {
        LOG_DEBUG(QString("serial number : ") + QString::fromStdString(sn));
        // staring normally, but defaults not set
        timing.setDefaultValues();
        currentStage.timestamp = current_timestamp();
        deviceConfig.serial_number = sn;
        deviceConfig.start_mode = 1;
        return 1;
    }
    // Staring first time setup
    setDefaultValues(uid);
    deviceConfig.start_mode = 2;

    return 2;
}

void php_hardware::getWiringsFromServer(cpuid_t uid)
{
    // TODO first send the request (sync, getWirings)
    // TODO Then update the wirings variable
    //  note that here may be multiple
}



