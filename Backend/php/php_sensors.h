#ifndef PHP_SENSORS_H
#define PHP_SENSORS_H

#include <QObject>
#include <QQmlEngine>

class php_sensors : public QObject
{
    Q_OBJECT
    //    QML_ELEMENT
public:
    explicit php_sensors(QObject *parent = nullptr);

/**
 * Retrieves the most recent data from the main sensor.
 * 
 * This method fetches the latest readings from the primary sensor that has not been removed (`is_main` is true and `is_remove` is false).
 * The fetched data includes temperature, humidity, time of flight (`tof`), ambient light (`ambiend`), and CO2 levels.
 * If the `settings` table indicates that the temperature should be measured in Fahrenheit (`measure_id` is 1), 
 * the method will convert the temperature value from Celsius to Fahrenheit.
 * 
 * @return array An array containing the most recent sensor readings. 
 *               The array structure is as follows:
 *               [ temperature, humidity, tof, ambient, co2 ]
 *               If no data is found, an empty array is returned.
 */
    void getSensorsData(void);
/**
 * Retrieves a list of paired sensors and the average temperature.
 * 
 * This method performs the following tasks:
 * 1. Fetches the measure unit (Celsius or Fahrenheit) from the settings table.
 * 2. Retrieves the current average temperature from the current_state table.
 * 3. Gets the list of all sensors that are paired and not marked as removed.
 * 4. If the measure unit is Fahrenheit, it converts the average temperature to Fahrenheit.
 * 5. Formats the sensor list to include only relevant details and converts some data types for consistency.
 * 
 * @return array An associative array containing two keys:
 *               - 'avg_temp': Represents the average temperature, either in Celsius or Fahrenheit 
 *                             based on the measure unit in the settings table.
 *               - 'sensors' : An array of sensors where each entry has the sensor's ID, its name, type, 
 *                             and a boolean indicating if it's the main sensor or not.
 */
    void getSensorList(void);
/**
 * Removes a sensor based on its ID.
 *
 * This method marks a sensor as removed in the database using its ID. It also updates 
 * the device configuration to indicate that a sensor has been forgotten.
 *
 * @param int $sensor_id The ID of the sensor to be removed.
 * 
 * @return bool Returns true if the operation was successful, false otherwise.
 */
    void remove(void);
/**
 * Retrieves a list of unpaired and non-main sensors from the database.
 * 
 * This method fetches all the sensors that meet the following criteria:
 * - They are not the primary/main sensors.
 * - They have not been paired yet.
 * - They have not been marked as removed.
 * 
 * Each entry in the list contains the sensor's ID, its name or description (sensor), and its type.
 * 
 * @return array An array containing the details of the sensors 
 *               (including their IDs, names/descriptions, and types). 
 *               If no matching sensors are found, it returns an empty array.
 */
    void getSensorPairList(void);
/**
 * Adds or edits a sensor's details.
 *
 * This method performs one of two tasks based on the provided sensor_id:
 * 1. If sensor_id is 0, it updates the details of a sensor to mark it as paired and assigns it 
 *    a location and name, then stops the pairing process on the device.
 * 2. If sensor_id is not 0, it updates an existing sensor's name and location.
 *
 * @param int $sensor_id The ID of the sensor. A value of 0 indicates the addition of a new sensor.
 * @param string $sensor The string identifier of the sensor, used when adding a new sensor.
 * @param string $name The name to be assigned to the sensor.
 * @param int $location_id The ID of the location to which the sensor should be assigned.
 * 
 * @return bool Returns true if the query was successful, otherwise false.
 */
    void setSensor(void);
/**
 * Checks if a given sensor name is unique in the database.
 *
 * This method checks the database to determine whether a given sensor name already exists. 
 * If the name exists, it returns false; if not, it returns true.
 *
 * @param string $name The name of the sensor to be checked.
 * 
 * @return bool Returns true if the given sensor name does not exist in the database, false otherwise.
 */
    void checkSensorName(void);
/**
 * Retrieves detailed information for a specified sensor.
 *
 * This method fetches the name, primary status, location, current temperature, and current humidity of a sensor based 
 * on its ID. It also considers the temperature measurement setting (Celsius or Fahrenheit) and converts the temperature 
 * value accordingly.
 *
 * @param int $sensor_id The ID of the sensor whose details are to be retrieved.
 * 
 * @return array Returns an associative array containing:
 *               - name: The name of the sensor.
 *               - is_main: Boolean indicating if the sensor is the primary one.
 *               - location_id: The ID of the location where the sensor is placed.
 *               - location_name: The name of the location where the sensor is placed.
 *               - temp: Current temperature reading of the sensor. The value will be '-' if no reading is available.
 *                      The temperature will be in Fahrenheit if the measure_id is 1, otherwise in Celsius.
 *               - humidity: Current humidity reading of the sensor. The value will be '-' if no reading is available.
 */
    void getSensorInfo(void);
/**
 * Retrieves a list of all sensor locations.
 * 
 * This method performs the following tasks:
 * 1. Fetches all the sensor locations from the sensor_locations table.
 * 2. Reformats the list to convert some data types for consistency.
 * 
 * @return array An array of sensor locations where each entry has:
 *               - 'id': The ID of the sensor location.
 *               - 'name': The name of the location.
 *               - 'alias': The alias (or shorthand) for the location.
 */
    void getSensorLocations(void);

/**
 * Initiates or terminates the device pairing mode based on the provided state.
 * 
 * This method updates the device configuration in the database to start or stop the pairing process.
 * If the provided state is `1`, the device is set to start pairing mode.
 * If the state is `0`, the pairing mode is terminated.
 * 
 * @param int $state An integer value representing the desired pairing state: 
 *                   1 to start pairing, and 0 to stop pairing.
 * @return bool Returns `true` if the database operation was successful, otherwise returns `false`.
 */
    void startEndPairing(void);

signals:

};

#endif // PHP_SENSORS_H
