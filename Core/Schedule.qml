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

    //! Whether this Schedule is active or not
    property bool       active: true

    //! Schedule type
    property string     type: ""

    //! Schedule temprature: This is always in Celsius
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
