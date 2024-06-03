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

    //! Mute alerts
    //! turn off the Alerts (excluding the embedded sensor malfunctions related ones)
    //! Note: The Embedded sensor malfunction related alerts are crucial
    //! so they should not be affected when User turns off the Alerts
    property bool       enabledAlerts:            false

    //! Turn off the notifications
    property bool       enabledNotifications:     false

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
