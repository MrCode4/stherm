import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * SchedulesController: create new Schedule and remove schedules.
 * ***********************************************************************************************/
QtObject {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property I_DeviceController deviceController

    property I_Device device: deviceController.device

    property var deviceCurrentSchedules: [];

    //! Deleting schedules: Use in retry operation
    //! Schedule id
    property var deletingSchedules: [];

    //! Editing schedules: Use in retry operation
    //! Schedule
    property var editingSchedules: [];

    //! Adding schedules: Use in retry operation
    //! Schedule
    property var addingSchedules: [];

    //! Run the timer schedule and update ui
    property bool runningScheduleEnabled: device.schedules.filter(schedule => schedule.enable).length > 0

    property bool scheduleEditing: deletingSchedules.length > 0 || addingSchedules.length > 0 || editingSchedules.length > 0

    onScheduleEditingChanged: {
        lockScheduleFetching();
    }

    /* Object properties
     * ****************************************************************************************/

    Component.onCompleted: device.schedulesChanged();

    /* Methods
     * ****************************************************************************************/

    function lockScheduleFetching(lock = false) {
        if (scheduleEditing || lock) {
            deviceController.updateLockMode(AppSpec.EMSchedule, true);

        } else {
            deviceController.updateLockMode(AppSpec.EMSchedule, false);
        }
    }

    //! Saves new schedule
    function saveNewSchedule(schedule: ScheduleCPP)
    {
        var newSchedule = cloneSchedule(schedule, AppCore.defaultRepo);

        // Update the created schedule with the current system mode
        setScheduleMode(newSchedule, device.systemSetup.systemMode);

        device.schedules.push(newSchedule);
        device.schedulesChanged();

        // Push new schedule to server and update schedule id
        addScheduleToServer(newSchedule)

        deviceController.saveSettings();

        return newSchedule;
    }

    //! Clone a schedule
    function cloneSchedule(schedule: ScheduleCPP, qsRepo = null) : ScheduleCPP {
        var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"],  AppCore.defaultRepo);
        newSchedule._qsRepo = qsRepo;
        newSchedule.enable = schedule.enable;
        newSchedule.name = schedule.name;
        newSchedule.type = schedule.type;
        newSchedule.minimumTemperature = schedule.minimumTemperature;
        newSchedule.maximumTemperature = schedule.maximumTemperature;
        newSchedule.humidity = schedule.humidity;
        newSchedule.startTime = schedule.startTime;
        newSchedule.endTime = schedule.endTime;
        newSchedule.repeats = schedule.repeats;
        newSchedule.dataSource = schedule.dataSource;
        newSchedule.systemMode = schedule.systemMode;

        return newSchedule;
    }

    //! Check if the entered schedule name is already being used in the existing schedule list.
    function isScheduleNameExist(name : string) : bool {
        let foundSchedule = device.schedules.find(sch => sch.name === name);

        return (foundSchedule !== undefined);
    }

    //! Remove an schedule
    function removeSchedule(schedule: ScheduleCPP)
    {
        var schIndex = device.schedules.findIndex(elem => elem._qsUuid === schedule._qsUuid);

        if (schIndex !== -1) {
            // Set repo to unregister deleted object
            schedule._qsRepo = null;

            // Send data to server
            clearScheduleFromServer(schedule.id);
            var schId = editingSchedules.findIndex(elem => elem.id === schedule.id);
            if (schId > -1) {
                editingSchedules.splice(schId, 1);
                editingSchedulesChanged();
            }

            schId = addingSchedules.findIndex(elem => elem.id === schedule.id);
            if (schId > -1) {
                addingSchedules.splice(schId, 1);
                addingSchedulesChanged();
            }


            device.schedules.splice(schIndex, 1);
            device.schedulesChanged();

//            schedule.destroy();

            deviceController.saveSettings();
        }
    }

    //! Return next day
    function nextDay(currentDay: string) : string {
        if (currentDay === "Mo") {
            return "Tu";

        } else if (currentDay === "Tu") {
            return "We";

        } else if (currentDay === "We") {
            return "Th";

        }  else if (currentDay === "Th") {
            return "Fr";

        } else if (currentDay === "Fr") {
            return "Sa";

        } else if (currentDay === "Sa") {
            return "Su";

        }  else if (currentDay === "Su") {
            return "Mo";
        }

        return "";
    }

    //! Prepare an string that keeps next days
    function nextDayRepeats(repeats: string) {
        var nextRepeats = [];
        repeats.split(",").forEach(elem => {
                        nextRepeats.push(nextDay(elem));
                        });

        return nextRepeats.join(",");
    }

    //! Assuming the date is same and just the time differs between variables
    function timeInRange(time: Date, schStartTime: Date, schEndTime: Date) {
        var todayTime = adjustDateToToday(time);
        var startTime = adjustDateToToday(schStartTime);
        var endTime   = adjustDateToToday(schEndTime);

        if (startTime < endTime) { // normal
            if (todayTime >= startTime && todayTime < endTime)
                return true;
        } else { // overnight
            if (todayTime >= startTime || todayTime < endTime)
                return true;
        }
        return false;
    }

    //! Convert the day to today
    function adjustDateToToday(time: Date) {
      // Create a new Date object for today
      let today = new Date();

      // Set the time of today to the time of the time object
      today.setHours(time.getHours());
      today.setMinutes(time.getMinutes());
      today.setSeconds(time.getSeconds());
      today.setMilliseconds(time.getMilliseconds());

      return today;
    }

    //! Find running days with repeat and start time.
    //! in repeat schedules is same as repeat.
    function findRunningDays(repeats: string, schStartTime: Date, schEndTime: Date, wasActive: bool ) {
        let scheduleRunningDays = repeats;

        // if no repeat find proper day based on current time and active state
        if (scheduleRunningDays.length === 0) {
            // scheduleRunningDays get from map
            var time = new Date();
            var isActive = timeInRange(time, schStartTime, schEndTime);

            if (!wasActive || !isActive){
                //! if startTime passed in this day!
                if (schStartTime < time)
                    time.setDate(time.getDate() + 1);
            } else {
                //! if overnight and time is the day after start
                if (schStartTime > time)
                    time.setDate(time.getDate() - 1);
            }

            scheduleRunningDays = Qt.formatDate(time, "ddd").slice(0, -1);
        }

        return scheduleRunningDays;
    }

    //! Finding overlapping Schedules, should be called before changing any time critial property
    function findOverlappingSchedules(startTimeStr, endTimeStr, repeats, exclude = null, active = false) {
        var overlappings = [];

        if (!device) return overlappings;

        var startTime = Date.fromLocaleTimeString(Qt.locale(), startTimeStr, "hh:mm AP")
        // if no repeat started ignore the startTime and get now as start to skip gone time
        if (repeats.length === 0 && active) {
            startTime = new Date();
        }
        var endTime = Date.fromLocaleTimeString(Qt.locale(), endTimeStr, "hh:mm AP")
        //! Start time seconds is 0 e.x. 05:05:00
        //! End time seconds is 59  e.x. 07:05:59
        endTime.setSeconds(59);

        // fix no repeat issue and update repeats
        let runningDays = findRunningDays(repeats, startTime, endTime, active);

        // over night, break into two schedules and call recursive for each
        if (endTime < startTime) {
            // active not important as running days are there for sure (already calculated)
            overlappings = findOverlappingSchedules(startTimeStr, "11:59 PM", runningDays, exclude);
            overlappings.push(findOverlappingSchedules("12:00 AM", endTimeStr, nextDayRepeats(runningDays), exclude));

            // return flatten array
            return overlappings.reduce((accumulator, value) => accumulator.concat(value), []);
        }

        // compare if there is any overlapping
        deviceCurrentSchedules.forEach(function(currentScheduleElement, index) {
            if (currentScheduleElement.scheduleElement === exclude ||
                    !currentScheduleElement.scheduleElement.enable) {
                return;
            }

            if (currentScheduleElement.runningDays.split(",").find((repeatDayElement, repeatIndex) => {
                                                             return runningDays.includes(repeatDayElement);
                                                         })) {
                var currentStartTime = currentScheduleElement.startTime;
                // if the current schedule is an active no repeat schedule ignore the past times
                if (currentScheduleElement.scheduleElement.repeats.length === 0 &&
                        currentScheduleElement.scheduleElement.active) {
                    var now = new Date();
                    var today = Qt.formatDate(now, "ddd").slice(0, -1);
                    // if today is repeat
                    if (today === currentScheduleElement.runningDays)
                        currentStartTime = now;
                    // else if today is next day ignore
                    else if (today === nextDayRepeats(currentScheduleElement.runningDays))
                        return;
                    // else, ....keep going, no need to do anything
                    // else if (currentScheduleElement.runningDays === nextDay(today));
                }

                // For accurate date comparisons, it's recommended to employ the .getTime() method to avoid potential issues.
                // e.x.: While `Sep 9 06:00:00 2024 GMT+0300` and `Mon Sep 9 06:00:00 2024 GMT+0300` appear different,
                // they represent the same timestamp (1725850800000).
                // getTime: The value returned by the getTime method is the number of milliseconds since 1 January 1970 00:00:00.
                if ((currentStartTime.getTime() > startTime.getTime() && currentStartTime.getTime() < endTime.getTime()) ||
                        (currentStartTime.getTime() < startTime.getTime() && currentScheduleElement.endTime.getTime() > startTime.getTime()) ||
                        currentStartTime.getTime() === startTime.getTime()) {
                    overlappings.push(currentScheduleElement.scheduleElement);
                }
            };
        })

        return overlappings;
    }

    //! it should be called after each change ASAP before findRunningSchedule called by timer
    function updateCurrentSchedules() {
        //! copy schedules into a simpler structure and fix the no repeat and overnight logics
        var currentSchedules = [];
        device.schedules.forEach(function(schElement, index) {
            var schStartTime = Date.fromLocaleTimeString(Qt.locale(), schElement.startTime, "hh:mm AP");
            // if no repeat started ignore the startTime and get now as start to skip gone time
            if (schElement.repeats.length === 0 && schElement.active) {
                schStartTime = new Date();
            }
            var schEndTime = Date.fromLocaleTimeString(Qt.locale(), schElement.endTime, "hh:mm AP");

            //! Start time seconds is 0 e.x. 05:05:00
            //! End time seconds is 59  e.x. 07:05:59
            schEndTime.setSeconds(59);
            // find the correct running day for no repeat
            let scheduleRunningDays = findRunningDays(schElement.repeats, schStartTime, schEndTime, schElement.active);
            var currentSchedule = {
                scheduleElement: schElement,
                startTime: schStartTime,
                endTime: schEndTime,
                runningDays: scheduleRunningDays
            }

            // check for overnight
            if (schEndTime > schStartTime)
            {
                currentSchedules.push(currentSchedule)
            }
            else // break into with correct running day
            {
                // Change to 11:59:59 PM
                currentSchedule.endTime = Date.fromLocaleTimeString(Qt.locale(), "11:59:59 PM", "hh:mm:ss AP");
                currentSchedules.push(currentSchedule)

                var currentScheduleNight = {
                    scheduleElement: schElement,
                    startTime: Date.fromLocaleTimeString(Qt.locale(), "12:00 AM", "hh:mm AP"),
                    endTime: schEndTime,
                    runningDays: nextDayRepeats(scheduleRunningDays)
                }
                currentSchedules.push(currentScheduleNight)
            }
        })

        deviceCurrentSchedules = currentSchedules;

        //! When the deviceCurrentSchedules changed, the running schedule should be check again.
        findRunningSchedule();
    }

    //! Find current schedule to active it and pass to Scheme to work around
    //! The function is triggered by the timer in the presence of active schedules.
    //! Additionally, it's called by the updateCurrentSchedules function to manage
    //! updates in scenarios where all schedules have been cleared.
    function findRunningSchedule() {
        var now = new Date();
        var currentDate = Qt.formatDate(now, "ddd").slice(0, -1);

        //! find the active schedule
        let currentSchedule = deviceCurrentSchedules.find(
                schedule => {
                    //! Compare time and running days to start it.
                    if (schedule.scheduleElement.enable &&
                        schedule.runningDays.includes(currentDate)) {
                         // logical compare would be, but in this case we miss one second for overnight schedules
                        if (timeInRange(now, schedule.startTime, schedule.endTime)) {
                            return true;
                        }
                    }

                    return false;
                });

        //! update the active state
        device.schedules.forEach(function(schElement, index) {
            var isActive = (schElement === (currentSchedule?.scheduleElement ?? null)) ?? false;
            // Disable a 'No repeat' schedule after running one time.
            if (schElement.repeats.length === 0 && schElement.active && !isActive) {
                schElement.enable = false;
            }

            schElement.active = isActive;
        })

        //! this function is called even if device is off or hold!
        //! We should no use a current schedule when device is on Hold, in Off Mode,
        //! in perf test or when emergency shut off!
        if (device.isHold || ((device?.systemSetup?.systemMode ?? AppSpec.Off) === AppSpec.Off || device.systemSetup.systemMode === AppSpec.EmergencyHeat) ||
                device?.systemSetup?._isSystemShutoff || PerfTestService.isTestRunning) {
            deviceController.setActivatedSchedule(null);
            return;
        }

        if (currentSchedule?.scheduleElement ?? false) {
            setScheduleNullTimer.stop();
            deviceController.setActivatedSchedule(currentSchedule.scheduleElement);

        } else {
            setScheduleNullTimer.start();
        }

    }

    //! Set the active schedule to null to prevent unintended changes around midnight
    //! when skipping the interval between 23:59:59 and 00:00:00.
    property Timer setScheduleNullTimer: Timer {
        interval: 1500
        repeat: false
        running: false
        onTriggered: {
            deviceController.setActivatedSchedule(null);
        }
    }

    //! Conver 24 hours to 12 hours format
    function formatTime(timeString) {
        // Split the time string into hours, minutes, and seconds
        const [hoursString, minutes, seconds] = timeString.split(':');

        // Convert hours to a number and handle leading zero
        const hours = parseInt(hoursString, 10);

        // Convert hours to 12-hour format and add AM/PM
        const amPm = hours >= 12 ? 'PM' : 'AM';
        const adjustedHours = hours % 12 || 12;  // Adjust for 12-hour format and noon

        // Format minutes with leading zero if needed
        const formattedMinutes = minutes.padStart(2, '0');

        // Return the formatted time string
        return `${adjustedHours.toString().padStart(2, '0')}:${formattedMinutes} ${amPm}`;
    }

    //! Conver 12 hours to 24 hours format
    function convert12HourTo24Hour(timeString) {
        const [time, period] = timeString.split(' ');

        const [hours, minutes] = time.split(':');
        let hour = parseInt(hours);

        if (period === 'PM' && hour !== 12) {
            hour += 12;
        } else if (period === 'AM' && hour === 12) {
            hour = 0;
        }

        const formattedTime = `${hour.toString().padStart(2, '0')}:${minutes}:00`;
        return formattedTime;
    }

    //! Compare the server schedules and the model schedules and update model based on the server data.
    function setSchedulesFromServer(serverSchedules: var) {

        if (scheduleEditing) {
            console.log("The schedules are being edited and cannot be updated by the server.")
            return;
        }

        console.log("Checking schedule from server.");

        var modelSchedules = device.schedules;
        if (!Array.isArray(serverSchedules)) {
            console.log("Invalid server input. Expected arrays.");
            return;
        }

        // Check the length of both arrays
        if (serverSchedules.length !== modelSchedules.length) {
            console.log("Number of schedules in server and model differ. ", serverSchedules.length,  modelSchedules.length);
        }

        // Clean the device schedules when the serverSchedules is empty.
        if (serverSchedules.length === 0) {
            console.log("Schedules in server is empty.");
            device.schedules = [];
            device.schedulesChanged();

            return;
        }

        var isNeedToUpdate = false;

        // Schedules that do not exist on the server will be deleted.
        modelSchedules.forEach(schedule => {
                                 // Find Schedule in the model
                                 var foundSchedule = serverSchedules.find(serverSchedule => schedule.id === serverSchedule.schedule_id);

                                 if (foundSchedule === undefined) {
                                     var schIndex = device.schedules.findIndex(elem => elem.id === schedule.id);
                                     if (schIndex !== -1) {
                                         device.schedules.splice(schIndex, 1);
                                         isNeedToUpdate = true;
                                     }
                                 }

                             });

        serverSchedules.forEach(schedule => {
                                  // Find Schedule in the model
                                    var foundSchedule = modelSchedules.find(modelSchedule => schedule.schedule_id === modelSchedule.id ||
                                                                            (modelSchedule.id  < 0 && schedule.name === modelSchedule.name)
                                                                            );

                                  // Add new schedule
                                    if (foundSchedule === undefined) {
                                        var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
                                        newSchedule._qsRepo = AppCore.defaultRepo;
                                        newSchedule.enable = schedule.is_enable;
                                        newSchedule.id = schedule.schedule_id;
                                        newSchedule.name = schedule.name;
                                        newSchedule.type = schedule.type_id;

                                        // TODO
                                        var difference = deviceController.temperatureUnit === AppSpec.TempratureUnit.Fah ? AppSpec.autoModeDiffrenceF : AppSpec.autoModeDiffrenceC

                                        newSchedule.minimumTemperature = Utils.clampValue(schedule?.auto_temp_low ?? AppSpec.defaultAutoMinReqTemp,
                                                                                          AppSpec.autoMinimumTemperatureC, AppSpec.autoMaximumTemperatureC - AppSpec.autoModeDiffrenceC);

                                        newSchedule.maximumTemperature = Utils.clampValue(schedule?.auto_temp_high ?? AppSpec.defaultAutoMaxReqTemp,
                                                                                          Math.max(newSchedule.minimumTemperature + AppSpec.autoModeDiffrenceC, AppSpec.minAutoMaxTemp),
                                                                                          AppSpec.autoMaximumTemperatureC);
                                        newSchedule.humidity = Utils.clampValue(schedule.humidity, AppSpec.minimumHumidity, AppSpec.maximumHumidity);
                                        newSchedule.startTime = formatTime(schedule.start_time);
                                        newSchedule.endTime = formatTime(schedule.end_time);
                                        newSchedule.repeats = schedule.weekdays.map(String).join(',');
                                        newSchedule.dataSource = schedule.dataSource;
                                        newSchedule.systemMode = (schedule?.mode_id - 1) ?? newSchedule.systemMode;

                                        device.schedules.push(newSchedule);

                                        isNeedToUpdate = true;
                                        console.log("Schecule: ", newSchedule?.id ?? "undefined", " added to model.");

                                    } else {
                                        foundSchedule.id = schedule.schedule_id ?? foundSchedule.id

                                        if (foundSchedule.enable !== schedule.is_enable) {
                                            foundSchedule.enable = schedule.is_enable;
                                        }

                                        if (foundSchedule.type !== schedule.type_id) {
                                            foundSchedule.type = schedule.type_id;
                                        }
                                        var startTime = formatTime(schedule.start_time);
                                        if (foundSchedule.startTime !== startTime) {
                                            foundSchedule.startTime = startTime;
                                        }
                                        var endTime = formatTime(schedule.end_time);
                                        if (foundSchedule.endTime !== endTime) {
                                            foundSchedule.endTime = endTime;
                                        }


                                        // Update schedule temperatures
                                        var autoTempLow = Utils.clampValue(schedule?.auto_temp_low ?? AppSpec.defaultAutoMinReqTemp,
                                                                           AppSpec.autoMinimumTemperatureC, AppSpec.autoMaximumTemperatureC - AppSpec.autoModeDiffrenceC);
                                        if (Math.abs(foundSchedule.minimumTemperature - autoTempLow) > 0.001) {
                                            foundSchedule.minimumTemperature = autoTempLow;
                                        }

                                        var autoTempHigh = Utils.clampValue(schedule?.auto_temp_high ?? AppSpec.defaultAutoMaxReqTemp,
                                                                            Math.max(foundSchedule.minimumTemperature + AppSpec.autoModeDiffrenceC, AppSpec.minAutoMaxTemp),
                                                                            AppSpec.autoMaximumTemperatureC);
                                        if (Math.abs(foundSchedule.maximumTemperature - autoTempHigh) > 0.001) {
                                            foundSchedule.maximumTemperature = autoTempLow;
                                        }

                                        // Update schedule mode
                                        var modeId = (schedule?.mode_id - 1) ?? foundSchedule.systemMode
                                        if (foundSchedule.systemMode !== modeId) {
                                            foundSchedule.systemMode = modeId;
                                        }

                                        if (foundSchedule.humidity !== schedule.humidity) {
                                            foundSchedule.humidity = schedule.humidity ?? 0;
                                        }

                                        if (foundSchedule.dataSource !== schedule.dataSource) {
                                            foundSchedule.dataSource = schedule.dataSource;
                                        }

                                        var repeats = schedule.weekdays.map(String).join(',');
                                        if (foundSchedule.repeats !== repeats) {
                                            foundSchedule.repeats = repeats;
                                        }

                                        console.log("Schecule: ", foundSchedule?.id ?? "undefined", " updated.");
                                    }
                                });

        if (isNeedToUpdate) {
            device.schedulesChanged();
        }
    }

    //Prepares a toast message to be shown upon addition or activation of a Schedule
    function prepareToastMessage(sch:ScheduleCPP):string{
        //Hold the prepared message which will be returned
        var toastMessage;
        var toastDetail;

        //Preparing some DateTime related variables
        var now = new Date();
        var currentDate = Qt.formatDate(now, "ddd").slice(0, -1);
        var scStartTime = Date.fromLocaleTimeString(Qt.locale(), sch.startTime, "hh:mm AP");
        var scEndTime = Date.fromLocaleTimeString(Qt.locale(), sch.endTime, "hh:mm AP");
        let runningDays = findRunningDays(sch.repeats, scStartTime, scEndTime, sch.active);

        // checks if the schecule is currently running
        let currentRunningDays = (scStartTime > scEndTime && scEndTime > now) ? nextDayRepeats(runningDays) : runningDays;
        if (currentRunningDays.includes(currentDate) && timeInRange(now, scStartTime, scEndTime)) {
            toastMessage = sch.name;
            toastDetail = " is already running!";
        }
        //otherwise, calculates the remaining time until the nearest time the schedule planned to run
        else{
            //first day from now on which the schedule is planned for
            var firstRunningDay = new Date(findNextDayofWeek(sch.startTime, runningDays));

            //Setting time as planned
            firstRunningDay.setHours(scStartTime.getHours());
            firstRunningDay.setMinutes(scStartTime.getMinutes());

            //Calcules the remaining time from now on
            var timeDifference = (firstRunningDay - now) ;

            // Formats the remaining time properly for displaying
            var minutes= Math.floor(timeDifference / 1000 / 60);
            var hours= Math.floor(minutes / 60);
            var days=Math.floor(hours / 24);

            minutes = minutes % 60;
            hours = hours % 24;

            //The remaining time as proper message
            toastMessage = sch.name;
            toastDetail = " will start in "
                    + ((days > 0) ? (days + " day" + (days > 1 ? "s " : " ")) : " ")
                    + ((hours > 0) ? (hours + " hour" + (hours > 1 ? "s " : " ")) : " ")
                    + minutes + " minutes ";
        }
        return { "message": toastMessage, "detail": toastDetail };
    }

    //Checks the repeating days of a schedule and finds the first day from now on
    function findNextDayofWeek(schStartTime, targetDays):Date
    {
        const schStartDtm = Date.fromLocaleTimeString(Qt.locale(), schStartTime, "hh:mm AP");
        const currentDate = new Date;
        var runningDays = targetDays.split(",");

        //! Check if start time has passed now. If it's not passed, next day of week might be the current day!
        //! For example when start time is 17:00 and now is 16:00 (schedule will start in one hour
        //! and next day of run is now)
        var startTimePassedNow = schStartDtm < currentDate;

        //Iterating over week days
        var i = startTimePassedNow ? 1 : 0;
        for (; i <= 7; ++i) {
            var nextDate = new Date;

            //Add one day to the current day in each iteration until a scheduled day is found
            nextDate.setDate(currentDate.getDate() + i);

            //Checks if the considered day is among the repeating days
            if (runningDays.includes(Qt.formatDate(nextDate, "ddd").slice(0, -1))){

                //returns the found date
                return nextDate;
            }
        }

        return null;
    }

    //! Deactivate schedules that are incompatible with the current system mode.
    function deactivateIncompatibleSchedules(checkWithSystemMode : int) {

        var incompatibleSchedules = findIncompatibleSchedules(checkWithSystemMode);
        incompatibleSchedules.forEach(schedule => {
                                          schedule.enable = false;
                                          editScheduleInServer(schedule);
                                      });

        // All incompatible schedules should be disabled befor call the updateSystemModeInCompatibleSchedules function.
        updateSystemModeInCompatibleSchedules(checkWithSystemMode);

        // Some schedules disabled, so we must safely update the current schedules.
        if (incompatibleSchedules.length > 0)
            device.schedulesChanged();
    }

    //! Find schedules that are incompatible with the current system mode.
    function findIncompatibleSchedules(checkWithSystemMode : int) {
        var incompatibleSchedules = device.schedules.filter(schedule =>
                                                        isScheduleIncompatible(schedule, checkWithSystemMode) &&
                                                        schedule.enable);

        return incompatibleSchedules;
    }

    //! Validate the compatibility of Scheduleds with the system mode.
    //! If the current system mode is cooling or heating, and it's changed
    //! to heating or cooling (respectively) or auto, the current schedule is incompatible.
    //! However, if the system mode is changed from
    //! auto to cooling or heating, the schedule will not be considered incompatible
    //! and will remain active.
    //! In the vacation and off mode schedule do not function so we keep the enabled schedule.
    //! Return true if the schedule is incompatible with the checkWithSystemMode.
    function isScheduleIncompatible(schedule: ScheduleCPP, checkWithSystemMode : int) : bool {

        return   schedule.systemMode !== checkWithSystemMode &&
                ((checkWithSystemMode === AppSpec.Cooling && schedule.systemMode === AppSpec.Heating) ||
                 ((checkWithSystemMode === AppSpec.Heating || checkWithSystemMode === AppSpec.EmergencyHeat) && schedule.systemMode === AppSpec.Cooling) ||
                 (checkWithSystemMode === AppSpec.Auto    && (schedule.systemMode === AppSpec.Cooling || schedule.systemMode === AppSpec.Heating || schedule.systemMode === AppSpec.EmergencyHeat)))
    }

    //! Update system mode of campatible schedules.
    function updateSystemModeInCompatibleSchedules(checkWithSystemMode : int) {
        device.schedules.filter(schedule => schedule.enable).forEach(schedule => {
                                                                        schedule.systemMode = checkWithSystemMode;
                                                                     });
    }

    function clearScheduleFromServer(id: int) {
        if (id < 0) {
            console.log("Remove schedule: The schedule is not exists in the server (unknown ID)");
            return;
        }

        var schId = deletingSchedules.findIndex(elem => elem === id);

        if (schId === -1) {
            deletingSchedules.push(id);
            deletingSchedulesChanged();
        }

        deviceController.sync.clearSchedule(id);
    }

    function editScheduleInServer(schedule: ScheduleCPP) {
        if (schedule.id < 0) {
            console.log("Edit schedule: The schedule is not exists in the server (unknown ID)");
            return
        }

        var schId = editingSchedules.findIndex(elem => elem.id === schedule.id);
        if (schId === -1) {
            editingSchedules.unshift(schedule);
            editingSchedulesChanged();
        }

        if (schedule) {
            var schedulePacket = schedulePacketServer(schedule);
            deviceController.sync.editSchedule(schedule.id, schedulePacket);
        }
    }

    function addScheduleToServer(schedule: ScheduleCPP) {
        var schId = addingSchedules.findIndex(elem => elem._qsUuid === schedule._qsUuid);
        if (schId === -1) {
            addingSchedules.unshift(schedule);
            addingSchedulesChanged();
        }

        if (schedule) {
            var schedulePacket = schedulePacketServer(schedule);
            deviceController.sync.addSchedule(schedule._qsUuid, schedulePacket);
        }

    }

    //! Prepare schedule packet for server
    function schedulePacketServer(schedule: ScheduleCPP) {
        var schedulePacket = {};
        schedulePacket.is_enable  = schedule.enable;
        schedulePacket.name       = schedule.name;
        schedulePacket.type_id    = schedule.type;
        schedulePacket.start_time = convert12HourTo24Hour(schedule.startTime);
        schedulePacket.end_time   = convert12HourTo24Hour(schedule.endTime);

        schedulePacket.mode_id = schedule.systemMode + 1;

        //  Send temp when it is single value mode and auto_temp_low and auto_temp_high when it is auto mode or others.
        if (schedule.systemMode === AppSpec.Cooling) {
            schedulePacket.temp       = schedule.maximumTemperature;

        } else if (schedule.systemMode === AppSpec.Heating) {
            schedulePacket.temp       = schedule.minimumTemperature;

        } else {
            schedulePacket.auto_temp_low  = schedule.minimumTemperature;
            schedulePacket.auto_temp_high = schedule.maximumTemperature;
        }

        schedulePacket.humidity   = schedule.humidity;
        schedulePacket.dataSource = schedule.dataSource;
        schedulePacket.weekdays   = schedule.repeats.split(',');

        return schedulePacket;
    }

    //! Set schedule mode
    //! If the system is Off, it will switch to Auto mode
    //! If the system is Emergency Heating, it will switch to Heating mode
    function setScheduleMode(schedule: ScheduleCPP, systemMode: int) {
        var sysMode = AppSpec.getScheduleModeWithSysMode(systemMode);

        // Update the created schedule with the current system mode
        schedule.systemMode = sysMode;
    }

    function clampRange(min, max, minDiff) {
        min = Utils.clampValue(min, deviceController._minimumTemperatureUI, deviceController._maximumTemperatureUI)
        max = Utils.clampValue(max, deviceController._minimumTemperatureUI, deviceController._maximumTemperatureUI)
        // Ensure max is greater than min by at least minDiff
        if (max <= min + minDiff) {
            max = min + minDiff; // Adjust max
        }

        // Re-clamp max to the range
        max = Math.min(deviceController._maximumTemperatureUI, max);

        // Adjust min if needed to ensure max > min + minDiff
        if (max <= min + minDiff) {
            min = max - minDiff;
        }

        return {"min": min, "max": max};
    }

    property Timer _checkRunningTimer: Timer {

        running: runningScheduleEnabled
        repeat: true
        interval: 1000

        onTriggered: {
            findRunningSchedule();
        }
    }

    property Connections deviceConnections: Connections {
        target: device

        function onIsHoldChanged() {
            console.log("device.isHold", device.isHold)
            if (device.isHold)
                deviceController.setActivatedSchedule(null);
        }

        //! Add/remove/enable/disable schedules
        function onSchedulesChanged() {
            updateCurrentSchedules();
        }
    }

    property Connections deviceControllerConnections: Connections {
        target: deviceController
        function onTemperatureUnitChanged() {
            unitChangeTimer.restart();
        }
    }

    //! to have the range updated
    property Timer unitChangeTimer: Timer {
        repeat: false
        running: false
        interval: 100
        onTriggered: {

           device.schedules.forEach(function(schElement, index) {

                if (deviceController.temperatureUnit === AppSpec.TempratureUnit.Cel){
                    let minC = schElement.minimumTemperature.toFixed(0);
                    let maxC = schElement.maximumTemperature.toFixed(0);
                    let minDiffC = schElement.systemMode === AppSpec.Auto ? AppSpec.autoModeDiffrenceC : 0;
                    let clampedC = clampRange(minC, maxC, minDiffC);

                    schElement.minimumTemperature = clampedC.min;
                    schElement.maximumTemperature = clampedC.max;

                } else {
                    let minF = Utils.convertedTemperature(schElement.minimumTemperature, AppSpec.TempratureUnit.Fah).toFixed(0)
                    let maxF = Utils.convertedTemperature(schElement.maximumTemperature, AppSpec.TempratureUnit.Fah).toFixed(0)
                    let minDiffF = schElement.systemMode === AppSpec.Auto ? AppSpec.autoModeDiffrenceF : 0;
                    let clampedF = clampRange(minF, maxF, minDiffF)

                    schElement.minimumTemperature =  Utils.fahrenheitToCelsius(clampedF.min);
                    schElement.maximumTemperature =  Utils.fahrenheitToCelsius(clampedF.max);
                }
            })
        }
    }

    property Connections deviceControllerCurrentConnections: Connections {
        target: deviceController.currentSchedule

        function onEnableChanged() {
            deviceController.setActivatedSchedule(null);
            updateCurrentSchedules();
        }

        function onIdChanged() {
            // Send the new id of current schedule
            if (deviceController.currentSchedule.id > -1)
                deviceController.updateEditMode(AppSpec.EMSchedule);
        }
    }


    //! Send null schedule when system mode changed to OFF mode
    property Connections systemSetupConnections: Connections{
        target: device?.systemSetup ?? null

        function on_IsSystemShutoffChanged() {
            if (device?.systemSetup?._isSystemShutoff)
                deviceController.setActivatedSchedule(null);
        }

        function onSystemModeChanged() {
            var currentSystemMode = device.systemSetup.systemMode;
            if (currentSystemMode === AppSpec.Off || currentSystemMode === AppSpec.EmergencyHeat) {
                deviceController.setActivatedSchedule(null);
            }

            if (currentSystemMode !== AppSpec.Off) {
                // Deactivate the incompatible schedules when mode changed from server or ui
                deactivateIncompatibleSchedules(currentSystemMode);
            }
        }
    }

    property Connections syncController: Connections {
        target: deviceController.sync

        function onScheduleCleared(id: int, success: bool) {
            var schId = deletingSchedules.findIndex(elem => elem === id);
            if (success) {
                console.log("Schedule cleared: ", id, schId);

                if (schId !== -1) {
                    deletingSchedules.splice(schId, 1);
                    deletingSchedulesChanged();
                }

            } else {
                // Schedule deleting failed, retry
                if (schId !== -1) {
                    deletingSchedules.push(schId);
                    deletingSchedulesChanged();
                }

                retryScheduleDeleting.start();
            }
        }

        function onScheduleEdited(id: int, success: bool) {
            var schId = editingSchedules.findIndex(elem => elem.id === id);
            if (success) {
                console.log("Schedule edited: ", id, schId);

                if (schId !== -1) {
                    editingSchedules.splice(schId, 1);
                    editingSchedulesChanged();
                }

            } else {
                // Schedule deleting failed, retry
                if (schId !== -1) {
                    editingSchedules.push(id);
                    editingSchedulesChanged();
                }

                retryScheduleEditing.start();
            }
        }

        function onScheduleAdded(scheduleUid, success: bool, schedule: var) {
            if (success) {
                var addedSchedule = device.schedules.find(elem => elem._qsUuid === scheduleUid);
                if (addedSchedule) {
                    console.log("addSchedule: schedule added with id: ", schedule.schedule_id)
                    addedSchedule.id = schedule.schedule_id;
                    deviceController.saveSettings();
                }

                var schId = addingSchedules.findIndex(elem => elem._qsUuid === scheduleUid);
                if (schId !== -1) {
                    addingSchedules.splice(schId, 1);
                    addingSchedulesChanged();
                }

            } else {
                retryScheduleAdding.start();
            }

        }
    }

    //! Retry to delete schedule
    property Timer retryScheduleDeleting: Timer {
        interval: 3000
        running: false
        repeat: false

        onTriggered: {
            if (deletingSchedules.length > 0)
                deviceController.sync.clearSchedule(deletingSchedules[0]);
        }
    }

    //! Retry to edit schedule
    property Timer retryScheduleEditing: Timer {
        interval: 4000
        running: false
        repeat: false

        onTriggered: {
            if (editingSchedules.length > 0)
                editScheduleInServer(editingSchedules[0]);
        }
    }

    //! Retry to add schedule
    property Timer retryScheduleAdding: Timer {
        interval: 2000
        running: false
        repeat: false

        onTriggered: {
            if (addingSchedules.length > 0)
                addScheduleToServer(addingSchedules[0]);
        }
    }
}
