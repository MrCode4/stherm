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
    property bool               on:             true

    //! Backlight color
    readonly property color     color:          Qt.hsva(hue, saturation, value)

    //! Hue of color
    property real               hue:            0.

    //! Saturation of color
    property real               saturation:     0.

    //! Value of color
    property real               value:          0.
}
