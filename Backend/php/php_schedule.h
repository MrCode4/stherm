#ifndef PHP_SCHEDULE_H
#define PHP_SCHEDULE_H

#include <QObject>
#include <QQmlEngine>

class php_schedule : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit php_schedule(QObject *parent = nullptr);

    void getScheduleList(void);
    void enableSchedule(void);
    void removeSchedule(void);
    void checkScheduleName(void);

    void getSchedule(void);
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
