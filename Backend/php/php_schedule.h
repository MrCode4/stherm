#ifndef PHP_SCHEDULE_H
#define PHP_SCHEDULE_H

#include <QObject>
#include <QQmlEngine>

class php_schedule : public QObject
{
    Q_OBJECT
    QML_ELEMENT
private:
/**
 * Retrieves schedules that have a time overlap with the provided schedule details.
 * 
 * This method is designed to fetch schedule IDs from the `schedules` table that
 * might potentially overlap with the given schedule details. It's important in
 * scenarios where there shouldn't be any conflicting schedules based on their timings
 * and days.
 * 
 * This method checks for several possible overlapping scenarios:
 * 1. The provided start time or end time is in between any existing schedule's start and end times.
 * 2. The provided start and end times completely envelope an existing schedule's timings.
 * 3. Both the provided and existing schedules have exactly the same start and end times.
 * 4. The existing schedule starts before midnight and ends after midnight, and overlaps with the provided timings.
 * 
 * Moreover, the method considers the days on which the schedules are active by checking
 * against the provided array of week days.
 * 
 * @param int $schedule_id The schedule ID to exclude from the search (typically the current schedule being checked).
 * @param string $start_time The start time of the schedule in 'HH:MM:SS' format.
 * @param string $end_time The end time of the schedule in 'HH:MM:SS' format.
 * @param array $week_days An indexed array of booleans representing the days of the week where `true` means the schedule is active on that day (starting from Sunday as 0).
 * @return array Returns an array of schedule IDs that potentially overlap with the provided details.
 */
    private function getMatchingSchedules(int $schedule_id, string $start_time, string $end_time, array $week_days); 




public:
    explicit php_schedule(QObject *parent = nullptr);

/**
 * Retrieves a list of schedules from the database.
 *
 * This method fetches a list of schedules from the `schedules` table where the schedules are not marked as removed.
 * It also groups and orders the result set based on various parameters.
 * 
 * The returned list of schedules also has its "state" value converted from 't'/'f' to true/false.
 * The method also typecasts certain values like 'id' and 'type' to integer for consistency.
 * Additionally, the 'week_days' field, which is fetched as a JSON array, is decoded to a PHP array.
 * 
 * @return array Returns an array of schedules, each schedule being an associative array.
 */
    void getScheduleList(void);
/**
 * Enables or disables a specific schedule based on the provided state.
 * 
 * This method is used to toggle the active state of a schedule in the `schedules` table.
 * When enabling a schedule (`$state` is true):
 *  - It first checks for any existing schedules that may conflict with the specified schedule's time.
 *  - If there are no conflicts, it enables the schedule.
 *  - If there are conflicts and `$other_schedule` is set to true, it doesn't make any changes and returns false.
 *  - If there are conflicts and `$other_schedule` is set to false, it disables all conflicting schedules and enables the specified schedule.
 * 
 * After enabling a schedule, it checks and possibly updates the current active schedule using the `setCurrentSchedule` method from the `System` class.
 * If `$state` is false, it simply disables the schedule.
 * 
 * @param int  $id             The ID of the schedule to be enabled/disabled.
 * @param bool $state          True to enable the schedule, false to disable.
 * @param bool $other_schedule If true, the method will not change other schedules that conflict with the specified schedule.
 *                             If false, it will disable all conflicting schedules.
 * @return bool                Returns true if the operation was successful, false otherwise.
 */
    void enableSchedule(void);
/**
 * Removes a specific schedule by marking it as removed in the `schedules` table.
 * 
 * This method does not physically delete the schedule from the database.
 * Instead, it marks the schedule as removed by updating the `is_remove` field to true.
 * Additionally, it marks the schedule as not sent by updating the `is_sent` field to false.
 * After marking the schedule as removed, it updates the current active schedule using the `setCurrentSchedule` method from the `System` class.
 * 
 * @param int $id The ID of the schedule to be removed.
 * @return bool Returns true if the operation was successful, false otherwise.
 */
    void removeSchedule(void);
/**
 * Checks whether a schedule name already exists in the `schedules` table.
 * 
 * This method verifies if a given schedule name already exists in the database.
 * If the name exists and is not marked as removed, the method returns false.
 * If the name doesn't exist or is an empty string, it returns true.
 * 
 * @param string $name The name of the schedule to be checked.
 * @return bool Returns true if the name does not exist in the database and is not an empty string. Returns false otherwise.
 */
    void checkScheduleName(void);

/**
 * Retrieves a specific schedule based on the provided schedule ID.
 * 
 * This method fetches details of a particular schedule from the `schedules` table 
 * based on the provided schedule ID, provided the schedule is not marked as removed.
 * 
 * The following transformations are performed on the fetched schedule:
 * - The 'id' and 'type' values are typecast to integers.
 * - The 'repeat' value, fetched as a JSON array, is decoded to a PHP array.
 * - The 'state' value is converted from 't'/'f' to true/false.
 * - The 'temperature' value is converted to Fahrenheit if the measure setting indicates so; otherwise, it's typecast to an integer.
 * 
 * @param int $id The schedule ID for which the details are to be retrieved.
 * @return array Returns an associative array representing the schedule's details.
 */
    void getSchedule(void);
/**
 * Sets or updates a schedule based on the provided details.
 * 
 * This method is designed to set or update a schedule in the `schedules` table.
 * The method includes several checks:
 * 1. The repeat parameter must have a length of 7 (one for each day of the week).
 * 2. The end time should be greater than the start time and the duration should be at least 10 minutes.
 * 3. If the temperature measure is in Fahrenheit, it is converted to Celsius.
 * 4. Depending on the schedule ID provided, it either updates an existing schedule or inserts a new one.
 * 5. Before inserting or updating a schedule, it checks for potential overlaps with existing schedules.
 * 
 * @param int $id The schedule ID (use 0 to add a new schedule).
 * @param string $name Name of the schedule.
 * @param int $temp The desired temperature for the schedule.
 * @param string $start_time The start time of the schedule in 'HH:MM:SS' format.
 * @param string $end_time The end time of the schedule in 'HH:MM:SS' format.
 * @param array $repeat An indexed array of integers (usually 0 or 1) representing each day of the week's status (0 for inactive and 1 for active).
 * @param int $type Type of the schedule.
 * @param bool $state State of the schedule (true for enabled and false for disabled).
 * @param bool $other_schedule Indicates whether to consider overlapping with other schedules.
 * @return mixed Returns true if successful, false if there's a conflict with other schedules, or an object with an error message if the time difference is less than 10 minutes.
 * @throws Exception if the repeat array does not have a length of 7.
 */
    void setSchedule(void);


/**
 * Fetches the current schedule based on a specific time.
 * 
 * This method determines the current schedule for a device based on the provided time
 * or the current timestamp from the `current_state` table if no time is provided.
 * 
 * The method works as follows:
 * 1. If no time is provided, fetch the current timestamp from the `current_state` table.
 * 2. Determine the day of the week based on the fetched or provided time.
 * 3. Use the day of the week to identify the corresponding column in the `schedules` table.
 * 4. Retrieve the temperature and schedule ID from the `schedules` table based on several conditions:
 *    - The schedule should not be removed.
 *    - The schedule should be enabled.
 *    - The schedule should be applicable for the determined day of the week.
 *    - The current timestamp should be greater than the `state_next_check` from the `current_state` table.
 *    - The provided or determined time should fall within the start and end times of the schedule.
 * 
 * @param string $time An optional time parameter in the format 'YYYY-MM-DD HH:MI:SS'.
 *                     If not provided, the current timestamp from the `current_state` table will be used.
 * 
 * @return array|null Returns an associative array containing the temperature and schedule ID 
 *                    if a matching schedule is found, or null otherwise.
 */



/**
 * Sets the next schedule in the `current_state` table.
 * 
 * This method determines the next schedule for a device and updates the `current_state` table
 * with the time when the next schedule should start.
 * 
 * The method works as follows:
 * 1. Fetch the `state_next_check` and `current_time` values from the `current_state` table.
 * 2. Determine the day of the week based on the fetched `state_next_check` or the current timestamp.
 * 3. Retrieve all the enabled schedules that are not removed from the `schedules` table, ordered by `start_time`.
 * 4. Loop through the schedules:
 *    - First, for the current day of the week and subsequent days until the end of the week.
 *    - If a schedule is not found, loop through the previous days of the week.
 * 5. Once the next schedule is identified, determine the number of days between the current day 
 *    and the day of the next schedule. Using this, calculate the timestamp of the next schedule.
 * 6. Update the `current_state` table with the `state_next_check` value as the timestamp of the next schedule.
 * 
 * @return bool Returns true if the `current_state` table is successfully updated, false otherwise.
 */


/**
 * Updates the current schedule in the `current_state` table.
 * 
 * This method determines the current schedule for a device and updates the `current_state` table
 * with the time when the current schedule starts. The logic is similar to `setNextSchedule`,
 * but it's focused on finding the current schedule instead of the next one.
 * 
 * The method works as follows:
 * 1. Fetch the `state_next_check` and `current_time` values from the `current_state` table.
 * 2. Determine the day of the week based on the fetched `state_next_check` or the current timestamp.
 * 3. Retrieve all the enabled schedules that are not removed from the `schedules` table, ordered by `start_time`.
 * 4. Loop through the schedules:
 *    - First, for the current day of the week and subsequent days until the end of the week.
 *    - If a schedule is not found, loop through the previous days of the week.
 * 5. Once the current schedule is identified, determine the number of days between the current day 
 *    and the day of the schedule. Using this, calculate the timestamp of the schedule.
 * 6. Update the `current_state` table with the `state_next_check` value as the timestamp of the current schedule.
 * 
 * @return bool Returns true if the `current_state` table is successfully updated, false otherwise.
 */




signals:

};

#endif // PHP_SCHEDULE_H
