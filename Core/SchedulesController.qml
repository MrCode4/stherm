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
    Component.onCompleted: {

        // No repeat schedules reset to default value ("No repeat")
        if (device)
            device.schedules.filter(elem => elem.type === AppSpec.Custom).forEach(function(element, index) {
                if (element.enable) {
                    return;
                }

                element.repeats = "No repeat";
            });
    }

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

    function nextDay(currentDay: string) : string {
        if (currentDay === "No repeat") {
            return "No repeat";

        } else if (currentDay === "Mo") {
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
    }

    function nextDayRepeats(repeats) {
        var nextRepeats = [];
        repeats.split(",").forEach(elem => {
                        nextRepeats.push(nextDay(elem));
                        });

        console.log("schStartTime r", nextRepeats.join(","))
        return nextRepeats.join(",");
    }

    //! Finding overlapping Schedules
    function findOverlappingSchedules(startTime: Date, endTime: Date, repeats, exclude = null)
    {
        if (!device) return [];

        var overlappings = [];
        if ((endTime - startTime) < 0) { // over night
            overlappings = findOverlappingSchedules(startTime, Date.fromLocaleTimeString(Qt.locale(), "11:59 PM", "hh:mm AP"), repeats, exclude);
            overlappings.push(findOverlappingSchedules(Date.fromLocaleTimeString(Qt.locale(), "12:00 AM", "hh:mm AP"),  endTime, nextDayRepeats(repeats), exclude));

            // return flatten array
            return overlappings.reduce((accumulator, value) => accumulator.concat(value), []);
        }

        device.schedules.forEach(function(element, index) {
            if (overlappings.includes(element))
                return;

            if (element === exclude || !element.enable) {
                return;
            }

            var schStartTime = Date.fromLocaleTimeString(Qt.locale(), element.startTime, "hh:mm AP");
            var schEndTime = Date.fromLocaleTimeString(Qt.locale(), element.endTime, "hh:mm AP");
            let currSchedule = {
                enable: true,
                name: element.name,
                type: element.type,
                startTime: schStartTime,
                endTime: schEndTime,
                repeats: element.repeats
            };

            // Break the over night schedule (move to next day)
            var currNightSchedule = null
            if ((schEndTime - schStartTime) < 0) {
                currSchedule.endTime = Date.fromLocaleTimeString(Qt.locale(), "11:59 PM", "hh:mm AP");

                currNightSchedule = {
                    enable: true,
                    name: element.name + "- over night",
                    type: element.type,
                    startTime: Date.fromLocaleTimeString(Qt.locale(), "12:00 AM", "hh:mm AP"),
                    endTime: schEndTime,
                    repeats: nextDayRepeats(element.repeats)
                };
            }

            const compare = (sch, startTime, endTime) => {
                return (sch.startTime > startTime && sch.startTime < endTime)
                    || (sch.endTime > startTime && sch.endTime < endTime)
                    || (startTime > sch.startTime && startTime < sch.endTime)
                    || (endTime > sch.startTime && endTime < sch.endTime)
                    || (startTime === sch.startTime && endTime === sch.endTime)
            }

            if (currSchedule.repeats.split(",").find((repeatElem, repeatIndex) => {
                                                    return repeats.includes(repeatElem);
                                                })) {
                if (compare(currSchedule, startTime, endTime)) {
                    overlappings.push(element);
                    return;
                }
            }

            if (currNightSchedule && currNightSchedule.repeats.split(",").find((repeatElem, repeatIndex) => {
                                                         return repeats.includes(repeatElem);
                                                     })) {
                if (compare(currNightSchedule, startTime, endTime)) {
                    overlappings.push(element);
                }
            }
        });

        return overlappings;
    }

    //! Find current schedule to active it and pass to Scheme to work around
    function findRunningSchedule() {

        let currentSchedule = null;

        if (!device.isHold &&
                (device?.systemSetup?.systemMode ?? AppSpec.Off) !== AppSpec.Off) {
            var now = new Date();

            var deviceSchedules = [];
            device.schedules.forEach(schedule => {
                                         if (!schedule.enable)
                                         return;

                                         if (currentSchedule)
                                         return;


                                         var schStartTime = Date.fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP");
                                         var schEndTime = Date.fromLocaleTimeString(Qt.locale(), schedule.endTime, "hh:mm AP");
                                         let currSchedule = {
                                             type: schedule.type,
                                             startTime: schStartTime,
                                             endTime: schEndTime,
                                             repeats: schedule.repeats
                                         };

                                         // Calculate over night schedules
                                         var currNightSchedule = null;

                                         if ((schEndTime - schStartTime) < 0) {
                                             currSchedule.endTime = Date.fromLocaleTimeString(Qt.locale(), "11:59 PM", "hh:mm AP");

                                             currNightSchedule = {
                                                 type: schedule.type,
                                                 startTime: Date.fromLocaleTimeString(Qt.locale(), "12:00 AM", "hh:mm AP"),
                                                 endTime: schEndTime,
                                                 repeats: nextDayRepeats(schedule.repeats)
                                             };
                                         }

                                         var currentDate = Qt.formatDate(new Date(), "ddd");
                                         currentDate = currentDate.slice(0, -1);

                                         console.log("findRunningSchedule Timer currNightSchedule.repeats",currSchedule.repeats,  currNightSchedule.repeats)
                                         if(currSchedule.repeats.includes(currentDate)) {


                                             if ((now >= currSchedule.startTime) && (now <= currSchedule.endTime)) {
                                                 currentSchedule = schedule;
                                                 return;
                                             }
                                         }


                                         if(currNightSchedule && currNightSchedule.repeats.includes(currentDate)) {


                                             if ((now >= currNightSchedule.startTime) && (now <= currNightSchedule.endTime)) {
                                                 currentSchedule = schedule;
                                             }
                                         }
                                     });
        }

        console.log("findRunningSchedule Timer", currentSchedule)

        // Disable a 'No repeat' schedule after running one time.
        if (runningSchedule !== currentSchedule && (runningSchedule?.repeats?.includes("No repeat") ?? false)) {
            runningSchedule.enable = false;
            runningSchedule.repeats = "No repeat";
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


    //! Check the custom/no repeat schedules.
    function checkNoRepeatSchedule(schedule: ScheduleCPP) {
        console.log("--- findRunningSchedule checkNoRepeatSchedule")
        if (schedule.type === AppSpec.Custom && schedule.repeats.includes ("No repeat")) {
            var now = new Date();
            var currentDate = Qt.formatDate(now, "ddd");

            var schStartTime = Date.fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP");
            if(schStartTime - now < 0) {
                now.setDate(schStartTime.getDate() + 1);
                currentDate = Qt.formatDate(now, "ddd");
            }
            console.log("--- findRunningSchedule checkNoRepeatSchedule", currentDate)

            currentDate = currentDate.slice(0, -1);
            schedule.repeats = ("No repeat," + currentDate);
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
