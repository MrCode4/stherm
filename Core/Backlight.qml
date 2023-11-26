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
    property bool               on:             false

    //! Backlight color, readonly for easier usage
    readonly property color     _color:          Qt.hsva(hue, saturation, value)

    //! Hue of color
    property real               hue:            0.

    //! Saturation of color
    property real               saturation:     1.

    //! Value of color
    property real               value:          1.

    //! Index of shade button in BacklightPage that backlight color is set based on it
    property int                shadeIndex:     0
}
