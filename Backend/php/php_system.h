#ifndef PHP_SYSTEM_H
#define PHP_SYSTEM_H

#include <QObject>
#include <QQmlEngine>

class php_system : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_system(QObject *parent = nullptr);

    /**
     * Fetches the system type from the device_config table.
     *
     * @return int The system type.
     */
        void getSystemType(void);
/**
 * Set the system type and accordingly configure the wirings.
 * 
 * The method starts by checking the current schedule. If none exists, it sets the next schedule. The system 
 * type can be set to traditional, heat pump, cool only, or heat only, and based on the type selected, it will 
 * configure the wiring table in the database.
 *
 * @param int $type System type. Can be one of the following:
 *                  1 - Traditional
 *                  2 - Heat Pump
 *                  3 - Cool Only
 *                  4 - Heat Only
 * 
 * @return bool True if the database queries are executed successfully, otherwise false.
 */
    void setSystemType(void);

    /**
     * Determines the number of traditional heating and cooling stages based on system type.
     *
     * @return array An associative array containing the number of heating and cooling stages.
     */
    void getTraditionalStages(void);
    /**
     * Configures the device's traditional heating and cooling stages.
     *
     * @param int $cool Number of cooling stages.
     * @param int $heat Number of heating stages.
     * @return bool True on successful update, False otherwise.
     */
    void setTraditionalStages(void);

    /**
     * Retrieves the number of heat pump stages, or defaults if none are set.
     *
     * @return array Associative array containing information about emergency heating, 
     *               OB state, emergency stages, and heat pump stages.
     * @throws Exception If any database-related issues occur.
     */
    void getHeatPumpStages(void);
    /**
     * Configures the heat pump stages based on provided parameters.
     *
     * @param int $emergency_heating Number indicating the emergency heating.
     * @param int $em_stages Number of emergency stages.
     * @param int $heat_pump_stages Number of heat pump stages.
     * @param string $ob_state OB state setting.
     * @return bool True if the operation was successful, false otherwise.
     * @throws Exception If any database-related issues occur.
     */
    void setHeatPumpStages(void);

    /**
     * Retrieves the current humidifier ID and wiring configuration.
     *
     * @return array Associative array containing the humidifier ID and wiring information.
     * @throws Exception If any database-related issues occur.
     */
    void getAccessories(void);
    /**
     * Configures the humidifier settings based on provided parameters.
     *
     * @param int $humidifier_id ID of the humidifier.
     * @param string $hum_wiring Humidifier wiring configuration.
     * @return bool True if the operation was successful, false otherwise.
     * @throws Exception If any database-related issues occur.
     */
    void setAccessories(void);

    /**
     * Fetches the system delay setting.
     *
     * @return int The current system delay in minutes.
     */
    void getSystemDelay(void);
        /**
     * Updates the system delay setting.
     *
     * @param int $min Desired system delay in minutes.
     * @return bool True if the operation was successful, false otherwise.
     * @throws Exception If any database-related issues occur.
     */
    void setSystemDelay(void);

    /**
     * Determines the number of cool stages based on system type.
     *
     * @return int Number of cool stages.
     */
    void getCoolStages(void);
    /**
     * Configures the device's cool stages.
     *
     * @param int $count Number of cool stages.
     * @return bool True on successful update, False otherwise.
     */
    void setCoolStages(void);

    /**
     * Fetches the number of heat stages based on wirings and system type.
     *
     * @return int The number of heat stages.
     */
    void getHeatStages(void);
    /**
     * Updates the heat stages in the wirings table based on the provided count.
     *
     * @param int $count The number of heat stages to set.
     * @return bool True if the operation was successful, false otherwise.
     * @throws Exception If any database-related issues occur.
     */
    void setHeatStages(void);

/**
 * Retrieves the available operational modes for the system.
 *
 * The function fetches available wirings from the database and determines 
 * the modes that can be enabled based on the wirings present.
 * 
 * The modes include:
 *   - cooling
 *   - heating
 *   - auto
 *   - vacation
 *
 * The function uses the presence or absence of specific wirings (like 'o/b', 'w1', 'y1') 
 * to determine which modes are enabled.
 * 
 * For instance:
 *   - If 'o/b' wiring is present, both cooling and heating modes are enabled.
 *   - If only 'y1' wiring is present, only the cooling mode is enabled.
 *   - If only 'w1' wiring is present, only the heating mode is enabled.
 *   - If both 'y1' and 'w1' are present, the auto mode is enabled.
 *
 * The default state for the modes is:
 *   - cooling: disabled
 *   - heating: disabled
 *   - auto: disabled
 *   - vacation: enabled
 *
 * @return array An array of mode aliases and their enabled status.
 */
    void getMode(void);
    
 /**
 * Updates the device mode in the current state and timing settings.
 * 
 * This method first checks the current schedule. If none exists, it will set the next schedule.
 * Subsequently, the method updates the mode in the `current_state` table and resets some timing parameters.
 * 
 * @param int $mode The desired mode to set. This corresponds to an ID that references a specific mode.
 * 
 * @return bool Returns true if the operation was successful, false otherwise.
 */
   void setMode(void);

/**
 * Fetches the ventilator setting from the device configuration.
 *
 * @return int Returns the value of the ventilator setting.
 */
    void getVentilator(void);


/**
 * Retrieves the current set humidity value and humidifier type from the database.
 *
 * This method fetches the desired humidity level and the type of the humidifier from the database.
 * After fetching, it typecasts the retrieved values to integer types to ensure consistent return data types.
 *
 * Here's a step-by-step breakdown of the method:
 * 1. Fetch the set humidity value and the humidifier type from the database.
 * 2. Typecast the `set_humidity` value to an integer.
 * 3. Typecast the `type` value (humidifier type) to an integer.
 * 
 * Note: Previously, there seems to be an attempt to map `type` based on a boolean value ('t' or 'f'). 
 * This code has been commented out, and the current implementation directly typecasts the fetched value to an integer.
 *
 * @return array Returns an associative array containing the set humidity value (`set_humidity`) and humidifier type (`type`).
 */
    void getHumidity(void);
/**
 * Sets the desired humidity level in the system.
 *
 * This method updates the desired humidity level in the database and ensures the system
 * is not set to "off" mode. If the system mode is "off", it changes the mode to "auto".
 * Before updating the value in the database, it checks to ensure that the desired humidity 
 * level is within a valid range (between 20% to 80%). If the provided humidity level is outside 
 * this range, an exception is thrown.
 *
 * Here's a step-by-step breakdown of the method:
 * 1. Check if the provided humidity value is within the allowed range (20% to 80%).
 * 2. If the value is outside this range, throw an exception.
 * 3. Begin constructing the SQL query to update the humidity level.
 * 4. Check the current mode of the system from the database.
 * 5. If the current mode is "off", append to the SQL query to update the mode to "auto".
 * 6. Execute the constructed SQL query and return the result.
 *
 * @param int $humidity The desired humidity level to be set.
 *
 * @throws Exception Throws an exception if the provided humidity value is outside the allowed range.
 *
 * @return bool Returns true if the operation was successful, false otherwise.
 */
    void setHumidity(void);

/**
 * Retrieves the current fan speed from the system.
 *
 * This method fetches the current speed of the fan from the database and returns it as an integer.
 * The value represents the speed at which the fan is currently operating, with 0 indicating the fan is off.
 * 
 * @return int Returns the current fan speed value.
 */
    void getFan(void);
/**
 * Sets the fan speed and status in the system.
 *
 * This method updates the fan speed and its status in the system based on the provided integer value.
 * The value for the fan speed should either be 0 or a multiple of 5. The function has the following behaviors:
 * 
 * 1. If the provided fan speed is 0:
 *    a. Update the fan speed in the database to 0.
 *    b. Set the fan_status to false (indicating the fan is off).
 * 
 * 2. If the provided fan speed is a multiple of 5:
 *    a. Update the fan speed in the database to the provided value.
 *    b. Set the fan_status to true (indicating the fan is on).
 *    c. Update the fan_time in the timing table to the current timestamp.
 *
 * If an incorrect value (neither 0 nor a multiple of 5) is provided, an exception is thrown.
 *
 * @param int $fan The desired fan speed value. Should be either 0 or a multiple of 5.
 *
 * @return bool Returns true if the operation was successful, false otherwise.
 *
 * @throws Exception If the provided fan speed is neither 0 nor a multiple of 5.
 */
    void setFan(void);

/**
 * Fetches main static data for the device, specifically the logo and the measure ID.
 *
 * The function retrieves the logo (referenced as 'img' in the returned associative array)
 * from the `device_config` table. Alongside the logo, it also fetches the measure ID from
 * the `settings` table. The measure ID is extracted from the first row in the `settings` table.
 *
 * @return array Returns an associative array with keys 'img' (representing the logo)
 *               and the measure ID from the `settings` table.
 */
    void getMainStatic(void);
/**
 * Fetches dynamic main data for the device.
 * 
 * This method retrieves various device-related data from the database, performs
 * several business logic operations, converts some of the data types, and finally returns
 * an array of the processed data.
 * 
 * 1. Check if the device needs an update and initiate it if necessary.
 * 2. Update the last_update timestamp in the device_config table.
 * 3. Retrieve multiple pieces of data related to the device, such as temperature, mode, 
 *    humidity, wifi signal strength, CO2 levels, and more.
 * 4. Process and convert certain data values.
 * 5. Check and update device states based on conditions.
 * 6. Convert temperature measurements between Celsius and Fahrenheit.
 * 7. Remove unnecessary data from the result set.
 * 
 * @throws Exception If any error occurs during the process.
 * 
 * @return array Returns an associative array containing the processed data.
 */
    void getMainData(void);
/**
 * Sets the temperature measurement unit for the system.
 *
 * This method allows the user to specify the temperature measurement unit they prefer.
 * The accepted values are:
 *   0 - Celsius
 *   1 - Fahrenheit
 *
 * @param int $measure An integer value representing the desired temperature measurement unit.
 *                     Accepted values: 0 (for Celsius) or 1 (for Fahrenheit).
 * 
 * @throws Exception Throws an exception if an incorrect value is provided.
 *
 * @return bool Returns true if the operation is successful, false otherwise.
 */
    void setMeasure(void);
/**
 * Marks a specified alert as read in the database.
 *
 * This function updates the `alerts` table by setting the status of a specified alert to `false`.
 *
 * @param int $id The ID of the alert to be marked as read.
 *
 * @return bool Result status of the query.
 */
    void setAlertAsRead(void);
/**
 * Retrieves the vacation settings from the database.
 * 
 * This function performs the following tasks:
 * 1. Queries the `vacation` table to fetch vacation related settings: 
 *    - minimum and maximum temperatures (`min_temp` and `max_temp`)
 *    - minimum and maximum humidity levels (`min_humidity` and `max_humidity`)
 *    - measure type (Fahrenheit or Celsius) from the `settings` table
 *    - humidifier type from the `device_config` table
 * 2. Depending on the measure type (Fahrenheit or Celsius), the temperatures are converted and adjusted.
 * 3. Constructs and returns an array of the vacation data.
 *
 * @return array $vacation_data Contains the minimum and maximum temperatures (potentially converted based on the measure type),
 *                              minimum and maximum humidity levels, and the humidifier type.
 * @throws Exception if the query doesn't return any vacation data.
 */
    void getVacationData(void);
/**
 * Toggles the vacation mode on or off.
 *
 * The function updates the `current_state` table based on the provided status.
 * If vacation mode is deactivated and no schedule is set, it returns the contractor's name.
 *
 * @param bool $status True to enable vacation mode, False to disable.
 * 
 * @return bool|string Result status of the query or contractor name.
 */
    void enableVacation(void);
/**
 * Sets vacation data in the database.
 * 
 * This function first determines the measurement system in use (Fahrenheit or Celsius).
 * It then converts temperatures if needed and updates the `vacation` and `current_state` tables.
 *
 * @param int $min_temp Minimum temperature.
 * @param int $max_temp Maximum temperature.
 * @param int $min_humidity Minimum humidity.
 * @param int $max_humidity Maximum humidity.
 * 
 * @return bool Result status of the query.
 * @throws Exception if the provided data does not meet the expected criteria.
 */
    void setVacation(void);
/**
 * Sets the system mode to 'off' and updates related states and configurations.
 * 
 * This function performs the following tasks:
 * 1. Calls `setNextSchedule` to set the next scheduled operation.
 * 2. Updates the `current_state` table in the database to set the mode to 'off'.
 * 3. Ensures that the message indicating the change in state hasn't been sent yet (`is_sent = false`).
 * 4. Sets the `state_id` to 0, which might indicate the system is now in an 'off' or neutral state.
 * 5. Updates the `relays` table in the database to turn off (set `type` to `false`) all the specified relays.
 *
 * @return bool Returns true if the database operations are successful, otherwise returns false.
 */
    void setOff(void);
/**
 * Retrieves all alerts from the database.
 *
 * This function queries the `alerts` table and joins with `alert_types` to get all alert data.
 * It then converts the ID and type fields to integers before returning the list of alerts.
 *
 * @return array List of alerts, with each alert containing its ID, name, and type.
 */
    void getAlerts(void);
/**
 * Fetches specific alert details from the database using its ID.
 * 
 * @param int $id The ID of the alert to fetch.
 * 
 * @return array Data of the specified alert, including its info (text), name, and type.
 */
    void getAlertData(void);
/**
 * Sets the desired temperature in the `current_state` table.
 *
 * This method updates the desired temperature in the `current_state` table and performs a number of
 * additional checks and actions based on the current state of the system.
 *
 * Here's how the method works:
 * 1. Fetch settings information which includes `measure_id`, `state_id`, and the current mode from the database.
 * 2. Depending on the measure system (Celsius or Fahrenheit):
 *    - Convert the temperature if necessary.
 *    - Validate the temperature range and throw an exception if out of range.
 * 3. Construct an SQL query to update the desired temperature in `current_state`.
 * 4. If the current mode is 'off', switch the mode to 'auto'.
 * 5. If there's no current schedule, set the next schedule. If there's a current schedule, reset some values in `current_state`.
 * 6. Update timestamps and other related fields in the `timing` table.
 * 7. Execute the constructed SQL query.
 *
 * @param int $temp The desired temperature value to be set.
 * 
 * @throws Exception if the temperature is out of range or if there's an error in the settings.
 * 
 * @return bool Returns true if the update query is executed successfully, false otherwise.
 */
    void setTemperature(void);
/**
 * Sets the hold status in the system.
 *
 * This method updates the hold status in the system based on the provided boolean value.
 * When the hold status is enabled (true), the system's current settings are maintained
 * and not overridden by scheduled changes. When the hold status is disabled (false),
 * the system may adjust settings based on scheduled configurations.
 *
 * Here's a step-by-step breakdown of the method:
 * 1. If the hold status is set to true:
 *    a. Update the hold_status in the database to true and mark the change as not sent.
 *    b. Check the current system state. If it's in state 2:
 *       i. Check the current schedule settings.
 *       ii. If a schedule exists, update the system temperature to match the scheduled temperature,
 *           change the mode to "auto", and reset the system state.
 * 2. If the hold status is set to false:
 *    a. Update the hold_status in the database to false and mark the change as not sent.
 *    b. Check if a current schedule exists. If it does, reset the system state.
 *
 * @param bool $hold The desired hold status. True for enabling hold, and false for disabling it.
 *
 * @return bool Returns true if the operation was successful, false otherwise.
 */
    void setHold(void);

/**
 * Fetches the technical access QR link based on the device's serial number.
 * If the device's serial number is not set, it will request the serial number from the server 
 * and subsequently update the device configuration with the received serial number.
 *
 * @return string Returns the full technical access QR link combined with the device's serial number.
 *                If the serial number cannot be determined, an empty string is returned.
 */
    void getQR(void);

    void getQRAnswer(void);

/**
 * Retrieves the UID and software/hardware versions of the device.
 *
 * This function fetches the UID from the `device_config` table and 
 * the software/hardware versions from the configuration file.
 *
 * @return array An array containing the software version, UID, and hardware version.
 */
    void getUidSV(void);
/**
 * Retrieves device configuration data for branding and integration.
 *
 * The function fetches details like logo, phone, URL, and ServiceTitan integration status
 * from the `device_config` table and returns them in an array.
 *
 * @return array An array containing the logo, phone, URL, and ServiceTitan status. Returns an empty array if no data is found.
 */
    void about(void);
/**
 * Requests a job from an external API.
 *
 * This function prepares a JSON payload that includes the device's serial number and the requested job type.
 * It then sends a POST request to an external web service. The result of the request is decoded and returned.
 *
 * @param string $type The type of job to be requested.
 *
 * @return bool|string Result of the job request. Returns false if the request fails or encounters an error.
 */
    void userGuide(void);
   /**
     * Fetches hardware and software versions, IP address, and the device's serial number.
     *
     * @return array An associative array containing hardware version, software version, IP address, and serial number.
     */
        void getVersionsIpSN(void);
/**
 * Shuts down the device.
 *
 * This function updates the `device_config` table setting the `shut_down` flag to true. 
 * (The commented line implies a system-level shutdown when uncommented).
 *
 * @return bool Result status of the database query.
 */
    void shutDown(void);

/**
 * Reboots the device.
 *
 * This function sends a system-level command to reboot the device.
 */
    void rebootDevice(void);
/**
 * Requests a job from an external API.
 *
 * This function prepares a JSON payload that includes the device's serial number and the requested job type.
 * It then sends a POST request to an external web service. The result of the request is decoded and returned.
 *
 * @param string $type The type of job to be requested.
 *
 * @return bool|string Result of the job request. Returns false if the request fails or encounters an error.
 */
    void requestJob(void);









/**
 * Fetches the technical edit link (QR code link) from the device configuration.
 *
 * @return string Returns the technical edit link.
 */

/**
 * Retrieves current data for testing purposes.
 *
 * The function fetches details like QA test status, current temperature, 
 * humidity, and average CO2 level from the database.
 *
 * @return array An array containing the current data.
 */

/**
 * Sets the current data for testing.
 *
 * This function updates the `device_config` and `current_state` tables with given values
 * for QA test status, temperature, humidity, and CO2 level.
 *
 * @param bool $qa_test QA Test status.
 * @param float $temp The temperature value.
 * @param int $humidity The humidity value.
 * @param int $co2 The CO2 level value.
 *
 * @return bool Result status of the database query.
 */



    /**
     * Retrieves the 'UPDATED' status from the device_settings.ini configuration file.
     *
     * @return int 1 if the device was updated, 0 otherwise.
     */



signals:

};

#endif // PHP_SYSTEM_H
