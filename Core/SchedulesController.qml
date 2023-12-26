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
    property I_Device device

    /* Methods
     * ****************************************************************************************/
    //! Saves new schedule
    function saveNewSchedule(schedule: ScheduleCPP)
    {
        var newSchedule = QSSerializer.createQSObject("ScheduleCPP", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
        newSchedule._qsRepo = AppCore.defaultRepo;
        newSchedule.name = schedule.name;
        newSchedule.type = schedule.type;
        newSchedule.temprature = schedule.temprature;
        newSchedule.humidity = schedule.humidity;
        newSchedule.startTime = schedule.startTime;
        newSchedule.endTime = schedule.endTime;
        newSchedule.repeats = schedule.repeats;
        newSchedule.dataSource = schedule.dataSource;

        device.schedules.push(newSchedule);
        console.log("sdsds" , newSchedule.repeats)
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
            if (element.repeats.find((repeatElem, repeatIndex) => {
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

    function findRunningSchedule() {
        device.schedules.forEach(schedule => {
                                     if (schedule.enable) {
                                         var currentDate = Qt.formatDate(new Date(), "ddd");
                                         currentDate = currentDate.slice(0, -1);
                                         console.log(schedule.repeats);

                                         if(schedule.repeats.includes(currentDate)) {
                                             var schStartTime = (Date).fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP");
                                             console.log(Qt.formatDate(new Date(), "ddd")  ,"-*-----", schedule.startTime,schStartTime, " --- ", schedule.endTime);
                                             var schEndTime = (Date).fromLocaleTimeString(Qt.locale(), schedule.endTime, "hh:mm AP");

                                             console.log(Qt.formatDate(new Date(), "ddd")  ,"-*-----", schedule.startTime, schEndTime," --- ", schedule.endTime, schEndTime);
                                         }
                                     }
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
