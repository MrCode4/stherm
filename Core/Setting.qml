import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * Setting: Keep all settings
 * ************************************************************************************************/

QSObject {
    //! Screen brightness: 0 to 100
    property int        brightness:             AppSpec.defaultBrightness

    //! Adaptive brightness
    property bool       adaptiveBrightness:     false

    //! Speaker volume: 0 to 100
    property int        volume:                 AppSpec.defaultVolume

    //! Temprature unit
    property int        tempratureUnit:         AppSpec.TempratureUnit.Fah

    //! Timer format
    property int        timeFormat:             AppSpec.TimeFormat.Hour12

    //! Current timezone
    property string     currentTimezone:        ""

    //! DST effect enable/disable
    property bool       effectDst:              true
}
