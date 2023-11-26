import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * Sensor model represents a physical sensor connected to the system
 * ***********************************************************************************************/
QSObject {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Name of this sensor
    property string     name:       ""

    //! Type of sensor (on-board or wireless
    property int        type:       AppSpec.SensorType.OnBoard

    //! Signal strength from 0, to 100
    property int        strength:   100

    //! Sensor battery level from 0 to 100
    property int        battery:    100

    //! Location of this senor
    property int        location:   AppSpec.SensorLocation.Unknown
}
