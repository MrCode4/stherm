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
    readonly property color     _color:         Qt.hsva(shadeIndex < 5 ? _whiteShade.hsvHue : hue,
                                                        shadeIndex < 5 ? shadeIndex / 4. : _saturation,
                                                        value)

    //! Saturation of color: is always 1 since saturation is not changable in backlight page
    readonly property real      _saturation:     1.

    //! Hue of color
    property real               hue:            0.

    //! Value of color
    property real               value:          1.

    //! Index of shade button in BacklightPage that backlight color is set based on it
    property int                shadeIndex:     5

    //! Shades of white which are predefined colors in backlight page
    readonly property var       _whiteShade:  Qt.color("#FF8200")
}
