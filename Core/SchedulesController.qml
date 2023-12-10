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
    function saveNewSchedule(schedule: Schedule)
    {
        var newSchedule = QSSerializer.createQSObject("Schedule", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
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
        device.schedulesChanged();
    }

    //! Remove an schedule
    function removeSchedule(schedule: Schedule)
    {
        var schIndex = device.schedules.findIndex(elem => elem === schedule);

        if (schIndex !== -1) {
            device.schedules.splice(schIndex, 1);
            device.schedulesChanged();

            schedule.destroy();
        }
    }

    //! Finding overlapping Schedules
    function findOverlappingSchedules(startTime: Date, endTime: Date, repeats)
    {
        if (!device) return [];

        var overlappings = [];
        device.schedules.forEach(function(element, index) {
            //! First check if repeats have at least one similiar values
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
}
