#ifndef PHP_HARDWARE_H
#define PHP_HARDWARE_H

#include <QObject>
#include <QQmlEngine>


#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "php/include/deviceconfig.h"
#include "php/include/timing.h"
#include "php/include/currentstage.h"
#include "php/include/sensors.h"

#include "UtilityHelper.h"


#ifdef _WIN32
#define uid_t uint8_t // for building in windows as test purpose
#endif

class php_hardware : public QObject
{
    Q_OBJECT
//    QML_ELEMENT

private:

    DeviceConfig &deviceConfig;
    Timing &timing;
    CurrentStage &currentStage;
    Sensors &sensors;


    ////////////////////// class variables adopted straight from php






    

    // TODO refactor these



    


/**
 * Sets default values in various tables for device configuration.
 *
 * This method initializes the database tables for device configuration by setting default values.
 * It first clears existing entries from the `device_config` and `timing` tables.
 * Then, it reads software and hardware version information from a configuration file and sets the
 * default values in the `device_config` and `timing` tables based on these versions and other predefined constants.
 *
 * @param string $uid The unique identifier used to initialize the `device_config` table.
 */
    void setDefaultValues(cpuid_t uid);

/**
 * Sets the system's timezone based on the `timezone_number` from the `device_config` table.
 *
 * This method fetches the `timezone_number` from the `device_config` table. If found, it attempts to set the
 * system's timezone by executing a command-line script. If the script execution is not successful or if the
 * `timezone_number` is not present, appropriate alerts are set for further action or review.
 */
    void setTimezone(void);

/**
 * Initializes the device settings and ensures the device is in a ready state.
 *
 * This method performs several tasks to set up the device for regular operation:
 * 1. Fetches the device's UID (Unique Identifier) and updates it in the database.
 * 2. If the device's UID is not found in the database or is empty, it sets default values.
 * 3. Updates certain timing and state parameters in the database.
 * 4. If a WiFi configuration file exists, reads the configuration, connects to the WiFi, and then deletes the configuration file.
 * 5. Cleans up any update files and directories.
 * 6. Sets the system's timezone.
 *
 * @return string Returns the UID of the device.
 */
    int runDevice(cpuid_t);
/**
 * Requests the serial number (SN) of a device using its UID (Unique Identifier).
 *
 * This method constructs a JSON payload to request the serial number (SN) for a device
 * with a given UID. The payload is sent to a remote server via the `sendCurlRequest` method
 * of the `sync` object.
 *
 * @param string $uid The UID of the device for which the SN is requested.
 * @return mixed Returns the result received from the remote server after sending the request.
 */
    int getSN(int uid);


public:
//    explicit php_hardware(QObject *parent = nullptr);
// TODO as the device config is used heavily here, we will pass a reference to this, so it can be managed externally
    explicit php_hardware(DeviceConfig &config, QObject *parent = nullptr);
    explicit php_hardware(DeviceConfig &config, Timing &tim, CurrentStage &stage, Sensors &sens, QObject *parent);

/**
 * Determines the starting mode of a device and performs initialization as necessary.
 *
 * This method performs several operations to establish the start mode of the device:
 * - Invokes the `runDevice` method to initialize device-specific settings.
 * - Executes a system command to retrieve the device's current start mode.
 * - Based on the device's start mode, various actions are taken:
 *   * PRODUCTION mode: Just updates the device_config table.
 *   * FIRST RUN or NORMAL mode: Checks if a device update is necessary, and if so, fetches wiring settings.
 *     Additionally, if the device doesn't have a serial number, it retrieves one from a remote server.
 *   * Unknown mode: Sets an alert for the system and defaults to PRODUCTION mode.
 *
 * @return int Returns 0 for PRODUCTION mode, 1 for NORMAL mode, or 2 for FIRST RUN mode.
 */
    int getStartMode(cpuid_t uid);


    // TODO move backlight to its own class?

    enum BacklightMode { on, slowBlink, fastBlink };

    /// @brief the state of the/a backlight
    struct Backlight
    {
        int             red;
        int             green;
        int             blue;
        BacklightMode   type;
        bool            onOff;
    };
    
/**
 * Updates the device's backlight configuration.
 *
 * This method performs the following operations:
 * - Converts the provided state (boolean) into an integer.
 * - Constructs an SQL query to update the device's backlight configuration, 
 *   specifically its RGB values, type, and state (on/off).
 * - Executes the query to update the `device_config` table with the provided values.
 * 
 * @param int  $R     Red component value (0-255) for backlight.
 * @param int  $G     Green component value (0-255) for backlight.
 * @param int  $B     Blue component value (0-255) for backlight.
 * @param int  $type  The type of the backlight.
 * @param bool $state The state of the backlight (true for ON, false for OFF).
 * @return bool       Returns true if the update was successful, otherwise returns false.
 */
    /**
     * @brief Control the backlight via nRF co-processor
     * 
     * TODO type should be more self explantory, e.g. bool blink, int blinkRate
     * TODO add a method or overload to turn off
     * 
     * @param red       red level, 0-255
     * @param green     green level, 0-255
     * @param blue      blue level, 0-255
     * @param type      0=on, 1=slow blink, 2=fast blink
     * @param onOff     true=on, false=off
     * @return true     success
     * @return false    failure
     */
    bool setBacklight(int red, int green, int blue, int type, bool onOff);

    /**
     * @brief Control the backlight via nRF co-processor
     * 
     * @param backlight backligh structure definition
     * @return true     success
     * @return false    failure
     */
    bool setBacklight(Backlight backlight);

/**
 * Retrieves the device's backlight configuration.
 *
 * This method performs the following operations:
 * - Fetches the backlight RGB values, type, and status from the `device_config` table.
 * - Processes the fetched RGB values to extract and convert them to individual integer components.
 * - Constructs an array containing the processed RGB values, backlight type, and status.
 * 
 * @return array An array containing:
 *               - Red component value (0-255) for backlight.
 *               - Green component value (0-255) for backlight.
 *               - Blue component value (0-255) for backlight.
 *               - The type of the backlight.
 *               - The state of the backlight (true for ON, false for OFF).
 */
    /**
     * @brief Return the current state of the backlight
     * 
     * TODO if variables are managed inside the UI, we shoudl be able to reference the Backlight variable
     * 
     * @return Backlight 
     */
    Backlight getBacklight(void);



    struct Wiring
    {
        bool    R;
        bool    C;
        bool    G;
        bool    Y1;
        bool    Y2;
        bool    ACC2;
        bool    ACC1P;
        bool    ACC1N;
        bool    W1;
        bool    W2;
        bool    W3;
        bool    OB;
    };

/**
 * Retrieves and checks the current wiring configurations.
 *
 * This method:
 * 1. Initiates a wiring check by updating the 'wiring_check' flag in the device_config table.
 * 2. Retrieves all wiring configurations from the database (from both wirings and wirings_temp tables).
 * 3. Compares the wirings configurations to determine if there are any mismatches.
 * 4. Sets an alert in case of wiring mismatches.
 * 5. Adjusts specific wirings based on logical conditions.
 * 6. Returns the list of current wirings configurations.
 *
 * @return mixed  An array of boolean values representing the state of each wiring configuration OR an object containing 
 *                a type, message, and result flag indicating specific wiring configurations issues.
 */
    // TBC, returns an array for 12 paramaters, one for each wiring
    // [R,C,G,Y1,Y2,ACC2,ACC1P,ACC1N,W1,W2,W3,OB]
    // The difference between this and below, is a bunch of data manipulations and database upadtes
    Wiring getWiring(void);
    
/**
 * Retrieves the actual wiring configurations from the system.
 *
 * This method fetches all wiring configurations from the database and returns them as an array of boolean values.
 * Each value in the array represents the state of a specific wiring: 
 *   - true indicates the wiring type is 't'.
 *   - false indicates the wiring type is not 't'.
 *
 * @return array  An array of boolean values representing the state of each wiring configuration in the order they are fetched from the database.
 */
    /**
     * @brief Get actual wiring from the database without requesting
     * 
     * TODO this naming needs refactoring, as the php description suggests this is the database version, and NOT the actual wiring data
     * 
     * @return Wiring 
     */
    Wiring getActualWiring(void);


/**
 * Checks the client's configuration based on the UID.
 *
 * This method:
 * 1. Retrieves the UID from the device_config table.
 * 2. Fetches the serial number (SN) using the retrieved UID.
 * 3. Validates the received SN. 
 * 4. If the SN is valid, updates the contractor information and sets the serial number in the device_config table.
 *
 * @return bool  True if the client's configuration is successfully validated and updated; otherwise, false.
 */
    /**
     * @brief Check if the device has a client
     * 
     * TODO despite the name this function will actually update the remote database via sync changContractorInfo
     * 
     * 
     * @return true 
     * @return false 
     */
    bool checkClient(void);


    /// @brief device configuration settings
    struct Settings
    {
        int     brightness;
        int     speaker;
        int     measure_id;
        int     time;
        bool    reset;
        bool    adaptive_brightness;    //!< true = adaptive, false = manual
        int     system_delay;
        int     measure;
    };
    

/**
 * Fetches and returns the device settings.
 *
 * This method retrieves device settings including:
 * - Brightness level
 * - Speaker volume
 * - Time setting (possibly time zone offset or similar time-related setting)
 * - Measurement system (e.g., metric vs. imperial)
 * - Adaptive brightness mode status
 *
 * All the values are cast to their respective types for consistency and clarity.
 *
 * @return array An associative array containing the various device settings.
 */
    Settings getSettings(void);

/**
 * Updates the device's settings.
 *
 * This method allows the user to configure various device settings including screen brightness, speaker volume, 
 * temperature measurement unit, time format, and adaptive brightness mode. It provides an option to reset all settings 
 * to their default values.
 *
 * @param int  $brightness           Desired brightness level (0 to 100).
 * @param int  $speaker              Desired speaker volume level (0 to 100).
 * @param int  $temp                 Temperature measurement unit (e.g., 1 for Fahrenheit).
 * @param int  $time                 Time format (0 for 12-hour format, 1 for 24-hour format).
 * @param bool $reset                If set to true, all settings are reset to default values.
 * @param bool $adaptive_brightness  Status of adaptive brightness mode (true for on, false for off).
 *
 * @return bool True if the settings were successfully updated in the database, false otherwise.
 */
    bool setSettings(Settings);


/**
 * Sets the device's screen brightness level and adaptive brightness mode.
 *
 * This method allows the user to set the desired brightness level for the device's screen.
 * It can also toggle the adaptive brightness mode on or off. The method first runs a system
 * command to actually set the device's brightness level. If the command is successful, the new
 * brightness level and the status of adaptive brightness mode are stored in the database.
 *
 * @param int  $brightness           Desired brightness level to set (likely a value between 0 and 100).
 * @param bool $adaptive_brightness  Status of adaptive brightness mode. True if on, false otherwise.
 *
 * @return bool True if the brightness was successfully set and stored, false otherwise.
 */
    /**
     * @brief Set the Brightness value and mode
     * 
     * calls the setBrightness executable.. TBC
     * 
     * TODO brightness value and adaptive settings could just be getter/setter
     * 
     * @param brightness    Brightness value
     * @param adaptive      true = adaptive, false = manual
     * @return true         success
     * @return false        failure
     */
    bool setBrightness(int brightness, bool adaptive);

/**
 * Fetches the wiring settings from a remote server based on a given UID and updates the local database.
 *
 * TODO what type is uid
 * 
 * This method performs the following operations:
 * - Constructs a JSON-encoded request payload, which includes the UID and other necessary parameters.
 * - Sends the request to the remote server via the `sendCurlRequest` method from the `sync` object.
 * - Processes the server's response to update the `wirings` table in the local database.
 * 
 * @param string $uid The unique identifier for the device to fetch wiring settings for.
 */
    void getWiringsFromServer(int uid);

/**
 * Sets an alert in the system based on the provided parameters.
 *
 * This method handles different types of alerts, including those from contractors and system-related issues.
 * If the same alert (by error code and source) was previously triggered within the last 24 hours,
 * it may not get re-triggered unless it's from a contractor.
 *
 * @param string $type       The type of alert (e.g., 'warning', 'error').
 * @param int    $sensor_id  The ID of the sensor related to the alert (0 if not related to any sensor).
 * @param int    $error_code A unique code identifying the nature of the error.
 * @param int    $level      Severity level of the alert.
 * @param string $name       A human-friendly name for the alert.
 * @param string $text       A description or message about the alert.
 * @param string $from       The source of the alert (e.g., 'system', 'contractor').
 *
 * @return bool  Returns true if the alert is successfully set or if the alert condition does not require setting,
 *               otherwise returns the result of the alert insertion into the database.
 */
    void setAlert(void);

/**
 * Retrieves the device's luminosity after setting the brightness and adaptive brightness mode.
 *
 * This method first sets the device's brightness and adaptive brightness mode based on the provided parameters. 
 * It then fetches the device's luminosity by executing an external command (`Luminosity.out`).
 * 
 * @param int  $brightness          The brightness level to set (0 to 100).
 * @param bool $adaptive_brightness Status of adaptive brightness mode (true for on, false for off).
 *
 * @return float|bool The luminosity of the device scaled down by a factor of 10 or false if an error occurs.
 */
    void getLuminosity(void);

signals:

};

#endif // PHP_HARDWARE_H
