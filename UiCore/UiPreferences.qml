import QtQuick
import Qt.labs.settings

import Stherm

/*! ***********************************************************************************************
 * UiPreferences holds settings related to application ui
 * ***********************************************************************************************/
QtObject {
    id: _root

    /* Enums
     * ****************************************************************************************/
    enum TempratureUnit {
        Cel,    //! Celsius
        Fah     //! Fahrenheit
    }

    enum TimeFormat {
        Hour12,
        Hour24
    }

    /* Property Declarations
     * ****************************************************************************************/
    //! Screen brightness: 0 to 100
    property int        brightness:             80

    //! Adaptive brightness
    property bool       adaptiveBrightness:     false

    //! Speaker volume: 0 to 100
    property int        volume:                 80

    //! Temprature unit
    property int        tempratureUnit:         UiPreferences.TempratureUnit.Fah

    //! Timer format
    property int        timeFormat:             UiPreferences.TimeFormat.Hour12

    // Whether the window is fullscreen or windowed
    property int        windowMode:             Window.Windowed

    //! Settings object
    property Settings   _settings:              Settings {
        category: "ui"

        property alias brightness:              _root.brightness
        property alias adaptiveBrightness:      _root.adaptiveBrightness
        property alias volume:                  _root.volume
        property alias tempratureUnit:          _root.tempratureUnit
        property alias timeFormat:              _root.timeFormat
        property alias windowMode:              _root.windowMode
    }

   function convertedTemperature(temp) {
       return tempratureUnit === UiPreferences.TempratureUnit.Fah ? 32 + 1.8 * temp  : temp
   }
}
