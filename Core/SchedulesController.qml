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
        var schIndex = device.schedules.findIndex(elem => elem === schedule);

        if (schIndex !== -1) {
            device.schedules.splice(schIndex, 1);
            device.schedulesChanged();

            schedule.destroy();
        }
    }

    //! Finding overlapping Schedules
    function findOverlappingSchedules(startTime: Date, endTime: Date, repeats, exclude=null)
    {
        if (!device) return [];

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

    // refactor
    //! Find current schedule to active it and pass to Scheme to work around
    function findRunningSchedule() {
        device.schedules.forEach(schedule => {
                                     var active = false;

                                    if (schedule.enable) {
                                        var currentDate = Qt.formatDate(new Date(), "ddd");
                                        currentDate = currentDate.slice(0, -1);

                                        if(schedule.repeats.includes(currentDate)) {
                                            let nowH = (new Date).getHours();
                                            let nowMin = (new Date).getMinutes();

                                            var startTime   = schedule.startTime.split(/[: ]/);
                                            var startTimeH  = parseInt(startTime[0]);
                                            var startTimeM  = parseInt(startTime[1]);
                                            var startPeriod = startTime[2].toUpperCase();

                                            if (startPeriod === "PM" && startTimeH !== 12) {
                                                startTimeH += 12;
                                            } else if (startPeriod === "AM" && startTimeH === 12) {
                                                startTimeH = 0;
                                            }

                                            var endTime   = schedule.endTime.split(/[: ]/);
                                            var endTimeH  = parseInt(endTime[0]);
                                            var endTimeM  = parseInt(endTime[1]);
                                            var endPeriod = endTime[2].toUpperCase();

                                            if (endPeriod === "PM" && endTimeH !== 12) {
                                                endTimeH += 12;
                                            } else if (endPeriod === "AM" && endTimeH === 12) {
                                                endTimeH = 0;
                                            }

                                            active = ((startTimeH < nowH) || (startTimeH === nowH && startTimeM <= nowMin)) &&
                                                                ((endTimeH > nowH) || (endTimeH === nowH && endTimeM >= nowMin));

                                        }
                                    }

                                    schedule._active = active;
                                    if (active)
                                       deviceController.setActivatedSchedule(schedule);
                                 });
    }

    property Timer _checkRunningTimer: Timer {
        running: device.schedules.filter(schedule => schedule.enable).length > 0
        repeat: true
        interval: 1000

        onTriggered: {
            findRunningSchedule();
        }

    }
}
