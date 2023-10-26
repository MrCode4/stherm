import QtQuick
import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * Backlight model holds data for device Backlight
 * ************************************************************************************************/
QSObject {
    /* Property declaration
     * ****************************************************************************************/
    //! Determines whether device backlight is on or off
    property bool       on:         true

    //! Backlight color
    property string     color:      "black"
}
