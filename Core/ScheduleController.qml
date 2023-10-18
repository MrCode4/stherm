import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ***********************************************************************************************/
QSObject {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    property var    schedules: []

    /* Methods
     * ****************************************************************************************/
    //! Saves new schedule
    function saveNewSchedule(schedule: Schedule)
    {
        var newSchedule = QSSerializer.createQSObject("Schedule", ["Stherm", ""], AppCore.defaultRepo);
        newSchedule._qsRepo = AppCore.defaultRepo;
        newSchedule.name = schedule.name;
        newSchedule.type = schedule.type;
        newSchedule.temprature = schedule.temprature;
        newSchedule.humidity = schedule.humidity;
        newSchedule.startTime = schedule.startTime;
        newSchedule.endTime = schedule.endTime;
        newSchedule.repeats = schedule.repeats;
        newSchedule.dataSource = schedule.dataSource;

        schedules.push(preset);
        schedulesChanged();
    }

    //! Remove an schedule
    function removeSchedule(schedule: Schedule)
    {
        var schIndex = schedules.findIndex(elem => elem === schedule);

        if (schIndex !== -1) {
            console.log('removed: ', schIndex);
            schedules.splice(schIndex, 1);
            schedulesChanged();

            schedule.destroy();
        }
    }
}
