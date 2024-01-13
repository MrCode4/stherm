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
    readonly property color     _color:         backlightFinalColor(shadeIndex, hue, value)

    //! Saturation of color: is always 1 since saturation is not changable in backlight page
    readonly property real      _saturation:    1.

    //! Shades of white which are predefined colors in backlight page
    readonly property var       _whiteShade:    Qt.color("#FF8200")

    //! Hue of color
    property real               hue:            0.

    //! Value of color (brightness, should not be less than 0.05, turn off for dark color)
    property real               value:          1.

    //! Index of shade button in BacklightPage that backlight color is set based on it.
    //! \see: BacklightPage
    property int                shadeIndex:     5

    //! This method is used to get color based on backlight logic. It is added here to avoid duplicate code
    //! \see: BacklightPage for more info
    function backlightFinalColor(shadeIndex, hue, value)
    {
        return Qt.hsva(shadeIndex < 5 ? _whiteShade.hsvHue : hue,
                       shadeIndex < 5 ? shadeIndex / 4. : _saturation,
                       value < 0.05 ? 0.05 : value);
    }
}
