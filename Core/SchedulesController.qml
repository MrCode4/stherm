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

    //! Finding overlapping Schedules
    function findOverlappingSchedules(startTime: Date, endTime: Date, repeats, exclude = null)
    {
        if (!device) return [];

        if ((endTime - startTime) < 0) {
            endTime.setDate(endTime.getDate() + 1);
        }
        var overlappings = [];
        device.schedules.forEach(function(element, index) {
            if (element === exclude || !element.enable) {
                return;
            }


            //! First check if repeats have at least one similar values
            if (element.repeats.split(",").find((repeatElem, repeatIndex) => {
                                     return repeats.includes(repeatElem);
                                 })) {
                var schStartTime = Date.fromLocaleTimeString(Qt.locale(), element.startTime, "hh:mm AP");
                var schEndTime = Date.fromLocaleTimeString(Qt.locale(), element.endTime, "hh:mm AP");

                if ((schEndTime - schStartTime) < 0) {
                    schEndTime.setDate(schEndTime.getDate() + 1);
                }

                if ((schStartTime > startTime && schStartTime < endTime)
                        || (schEndTime > startTime && schEndTime < endTime)
                        || (startTime > schStartTime && startTime < schEndTime)
                        || (endTime > schStartTime && endTime < schEndTime)) {
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

            device.schedules.forEach(schedule => {
                                        if (currentSchedule)
                                            return;

                                        var isRunning = false;

                                        if (schedule.enable) {
                                            var currentDate = Qt.formatDate(new Date(), "ddd");
                                            currentDate = currentDate.slice(0, -1);

                                            if(schedule.repeats.includes(currentDate)) {

                                                var schStartTime = Date.fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP");
                                                var schEndTime = Date.fromLocaleTimeString(Qt.locale(), schedule.endTime, "hh:mm AP");

                                                if ((schEndTime - schStartTime) < 0) {
                                                    schEndTime.setDate(schEndTime.getDate() + 1);
                                                }

                                                isRunning = (now >= schStartTime) && (now <= schEndTime);
                                            }
                                        }

                                        if (isRunning)
                                            currentSchedule = schedule;
                                     });
        }

        deviceController.setActivatedSchedule(currentSchedule);
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

    property Connections deviceConnections: Connections{
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

    property Connections deviceControllerConnection: Connections {
        target: deviceController
    }
}
