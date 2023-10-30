import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * Sensor model represents a physical sensor connected to the system
 * ***********************************************************************************************/
QSObject {
    id: _root

    /* Enums
     * ****************************************************************************************/
    enum Location {
        Unknown,
        Bedroom,
        LivingRoom,
        Kitchen
    }

    /* Property declaration
     * ****************************************************************************************/
    //! Name of this sensor
    property string     name:       ""

    //! Location of this senor
    property int        location:   Sensor.Location.Unknown
}
