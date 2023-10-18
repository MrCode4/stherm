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

        schedules.push(preset);
        schedulesChanged();
    }
}
