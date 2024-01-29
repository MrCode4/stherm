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
    }

    //! Remove an schedule
    function removeSchedule(schedule: ScheduleCPP)
    {
        var schIndex = device.schedules.findIndex(elem => elem._qsUuid === schedule._qsUuid);

        if (schIndex !== -1) {
            device.schedules.splice(schIndex, 1);
            device.schedulesChanged();

//            schedule.destroy();
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

        if (!device._isHold) {
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

    property Timer _checkRunningTimer: Timer {
        running: !device._isHold && device.schedules.filter(schedule => schedule.enable).length > 0
        repeat: true
        interval: 1000

        onTriggered: {
            findRunningSchedule();
        }

    }

    property Connections deviceConnections: Connections{
        target: device

        function on_IsHoldChanged() {
            console.log("device._isHold", device._isHold)
            if (device._isHold)
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
}
