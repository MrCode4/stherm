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

    property ScheduleCPP runningSchedule: null

    /* Object properties
     * ****************************************************************************************/

    /* Methods
     * ****************************************************************************************/
    //! Saves new schedule
    function saveNewSchedule(schedule: ScheduleCPP)
    {
        var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
        newSchedule._qsRepo = AppCore.defaultRepo;
        newSchedule.enable = schedule.enable;
        newSchedule.name = schedule.name;
        newSchedule.type = schedule.type;
        newSchedule.temprature = schedule.temprature;
        newSchedule.humidity = schedule.humidity;
        newSchedule.startTime = schedule.startTime;
        newSchedule.endTime = schedule.endTime;
        newSchedule.repeats = schedule.repeats;
        newSchedule.dataSource = schedule.dataSource;

        device.schedules.push(newSchedule);
        device.schedulesChanged();

        // Send data to server and save file
        deviceController.pushSettings();
    }

    //! Remove an schedule
    function removeSchedule(schedule: ScheduleCPP)
    {
        var schIndex = device.schedules.findIndex(elem => elem._qsUuid === schedule._qsUuid);

        if (schIndex !== -1) {
            device.schedules.splice(schIndex, 1);
            device.schedulesChanged();

//            schedule.destroy();

            // Send data to server
            deviceController.pushSettings();
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

    //! Find running days with repeat and start time.
    //! in repeat schedules is same as repeat.
    function findRunningDays(repeats: string, schStartTime: Date) {
        let scheduleRunningDays = repeats;

        // if no repeat find proper day based on current time
        if (scheduleRunningDays.length === 0) {
            var now = new Date();

            if (schStartTime < now)
                now.setDate(now.getDate() + 1);

            scheduleRunningDays = Qt.formatDate(now, "ddd").slice(0, -1);
        }

        return scheduleRunningDays;
    }

    //! Finding overlapping Schedules
    function findOverlappingSchedules(startTime: Date, endTime: Date, repeats, exclude = null)
    {
        var overlappings = [];

        if (!device) return overlappings;

        // fix no repeat issue and update repeats
        let runningDays = findRunningDays(repeats, startTime);

        // over night, break into two schedules and call recursive for each
        if (endTime < startTime) {
            overlappings = findOverlappingSchedules(startTime, Date.fromLocaleTimeString(Qt.locale(), "11:59 PM", "hh:mm AP"), runningDays, exclude);
            overlappings.push(findOverlappingSchedules(Date.fromLocaleTimeString(Qt.locale(), "12:00 AM", "hh:mm AP"),  endTime, nextDayRepeats(runningDays), exclude));

            // return flatten array
            return overlappings.reduce((accumulator, value) => accumulator.concat(value), []);
        }

        // copy schedules into a simpler structure and fix the no repeat and overnight logics
        var currentSchedules = [];
        device.schedules.forEach(function(schElement, index) {
            if (schElement === exclude || !schElement.enable) {
                return;
            }

            var schStartTime = Date.fromLocaleTimeString(Qt.locale(), schElement.startTime, "hh:mm AP");
            var schEndTime = Date.fromLocaleTimeString(Qt.locale(), schElement.endTime, "hh:mm AP");
            // find the correct running day for no repeat
            let scheduleRunningDays = findRunningDays(schElement.repeats, schStartTime);

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
                currentSchedule.endTime = Date.fromLocaleTimeString(Qt.locale(), "11:59 PM", "hh:mm AP");
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

        // compare if there is any overlapping
        currentSchedules.forEach(function(currentScheduleElement, index) {
            if (currentScheduleElement.runningDays.split(",").find((repeatDayElement, repeatIndex) => {
                                                             return runningDays.includes(repeatDayElement);
                                                         })) {
                if ((currentScheduleElement.startTime > startTime && currentScheduleElement.startTime < endTime) ||
                        (currentScheduleElement.startTime < startTime && currentScheduleElement.endTime > startTime) ||
                        currentScheduleElement.startTime === startTime) {
                    overlappings.push(currentScheduleElement.scheduleElement);
                }
            };
        })

        return overlappings;
    }

    //! Find current schedule to active it and pass to Scheme to work around
    function findRunningSchedule() {

        let currentSchedule = null;

        if (!device.isHold &&
                (device?.systemSetup?.systemMode ?? AppSpec.Off) !== AppSpec.Off) {

            // Lambda compare: compare the current schedule time and NOW
            const compare = (sch) => {
                var currentDate = Qt.formatDate(new Date(), "ddd");
                currentDate = currentDate.slice(0, -1);

                if (sch.runningDays.includes(currentDate)) {
                     var now = new Date().getTime();
                    if ((now >= sch.startTimeStamp) && (now <= sch.endTimeStamp)) {
                        return true;
                    }

                    return false;
                }
            }

            var deviceSchedules = [];
            device.schedules.every(schedule => {
                                         if (!schedule.enable)
                                         return true;

                                         var schStartTimeStamp = Date.fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP").getTime();
                                         var schEndTimeStamp = Date.fromLocaleTimeString(Qt.locale(), schedule.endTime, "hh:mm AP").getTime();

                                         // Schedule run in days.
                                         let scheduleRunningDays = findRunningDays(schedule.repeats, schStartTimeStamp);

                                         // **** Prepare the schedule to compare ****
                                         let currSchedule      = null;
                                         let currNightSchedule = null;

                                         {
                                             // Copy the current schedule into currSchedule and change it when necessary
                                             currSchedule = {
                                                 type: schedule.type,
                                                 startTimeStamp: schStartTimeStamp,
                                                 endTimeStamp: schEndTimeStamp,
                                                 runningDays: scheduleRunningDays
                                             };

                                             // Calculate over night schedule
                                             if ((schEndTimeStamp - schStartTimeStamp) < 0) {
                                                 currSchedule.endTimeStamp = Date.fromLocaleTimeString(Qt.locale(), "11:59 PM", "hh:mm AP").getTime();

                                                 currNightSchedule = {
                                                     type: schedule.type,
                                                     startTimeStamp: Date.fromLocaleTimeString(Qt.locale(), "12:00 AM", "hh:mm AP").getTime(),
                                                     endTimeStamp: schEndTimeStamp,

                                                     // Move running days into next days
                                                     runningDays: nextDayRepeats(scheduleRunningDays)
                                                 };
                                             }
                                         }

                                         //! Compare time and running days to start it.
                                         if(compare(currSchedule)) {
                                             currentSchedule = schedule;
                                             return false;
                                         }

                                         if(currNightSchedule && compare(currNightSchedule)) {
                                             currentSchedule = schedule;
                                             return false;
                                         }
                                     });
        }

        // Disable a 'No repeat' schedule after running one time.
        if (runningSchedule !== currentSchedule && ((runningSchedule?.repeats.length === 0) ?? false)) {
            runningSchedule.enable = false;
        }

        runningSchedule = currentSchedule;
        deviceController.setActivatedSchedule(runningSchedule);
    }

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

    //! Compare the server schedules and the model schedules and update model based on the server data.
    function setSchedulesFromServer(serverSchedules: var) {

        var modelSchedules = device.schedules;
        if (!Array.isArray(serverSchedules)) {
            console.log("Invalid server input. Expected arrays.");
            return;
        }

        // Check the length of both arrays
        if (serverSchedules.length !== modelSchedules.length) {
            console.log("Number of schedules in server and model differ.");
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
        modelSchedules.every(schedule => {
                                 // Find Schedule in the model
                                 var foundSchedule = serverSchedules.find(serverSchedule => schedule.name === serverSchedule.name);

                                 if (foundSchedule === undefined) {
                                     var schIndex = device.schedules.findIndex(elem => elem.name === schedule.name);
                                     if (schIndex !== -1) {
                                         device.schedules.splice(schIndex, 1);
                                         isNeedToUpdate = true;
                                     }
                                 }

                             });

        serverSchedules.every(schedule => {
                                  // Find Schedule in the model
                                  var foundSchedule = modelSchedules.find(modelSchedule => schedule.name === modelSchedule.name);

                                  // Add new schedule
                                  if (foundSchedule === undefined) {
                                      var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
                                      newSchedule._qsRepo = AppCore.defaultRepo;
                                      newSchedule.enable = schedule.is_enable;
                                      newSchedule.name = schedule.name;
                                      newSchedule.type = schedule.type_id;
                                      newSchedule.temprature = schedule.temp;
                                      newSchedule.humidity = schedule.humidity;
                                      newSchedule.startTime = formatTime(schedule.start_time);
                                      newSchedule.endTime = formatTime(schedule.end_time);
                                      newSchedule.repeats = schedule.weekdays.map(String).join(',');
                                      newSchedule.dataSource = schedule.dataSource;

                                      device.schedules.push(newSchedule);

                                      isNeedToUpdate = true;

                                  } else {
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
                                      var endTime = formatTime(schedule.end_time)
                                      if (foundSchedule.endTime !== endTime) {
                                          foundSchedule.endTime = endTime;
                                      }

                                      if (foundSchedule.temprature !== schedule.temp) {
                                          foundSchedule.temprature = schedule.temp;
                                      }

                                      if (foundSchedule.humidity !== schedule.humidity) {
                                          foundSchedule.humidity = schedule.humidity;
                                      }

                                      if (foundSchedule.dataSource !== schedule.dataSource) {
                                          foundSchedule.dataSource = schedule.dataSource;
                                      }

                                      var repeats = schedule.weekdays.map(String).join(',')
                                      if (foundSchedule.repeats !== repeats) {
                                          foundSchedule.repeats = repeats;
                                      }
                                  }
                              });

        if (isNeedToUpdate) {
            device.schedulesChanged();
        }
    }

    property Timer _checkRunningTimer: Timer {
        running: (device?.systemSetup?.systemMode ?? AppSpec.Off) !== AppSpec.Off &&
                 !device.isHold && device.schedules.filter(schedule => schedule.enable).length > 0
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
    }

    property Connections deviceControllerConnections: Connections{
        target: deviceController.currentSchedule

        function onEnableChanged() {

            deviceController.setActivatedSchedule(null);
            findRunningSchedule();
        }
    }


    //! Send null schedule when system mode changed to OFF mode
    property Connections systemSetupConnections: Connections{
        target: device.systemSetup

        function onSystemModeChanged() {
            if ((device?.systemSetup?.systemMode ?? AppSpec.Off) === AppSpec.Off) {
                deviceController.setActivatedSchedule(null);
            }
        }
    }
}
