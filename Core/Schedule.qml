import QtQuick
import QtQuickStream

import Stherm

/*! ***********************************************************************************************
 * Schedule is a model for holding schedule information
 * ***********************************************************************************************/
QSObject {
    /* Property declaration
     * ****************************************************************************************/
    //! Schedule name
    property string     name: ""

    //! Schedule type
    property string     type: ""

    //! Schedule temprature
    property real       temprature: 0.

    //! Schedule humidity
    property real       humidity: 0.

    //! Schedule start time
    property string     startTime: ""

    //! Schedule end time
    property string     endTime: ""

    //! Schedule repeat
    property var        repeats: []

    //! Schedule data source -> A sensor
    property string     dataSource: ""
}
