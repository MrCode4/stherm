import QtQuick
import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 *
 * ************************************************************************************************/
QSObject {
    id: appModel

    /* Property Declarations
     * ****************************************************************************************/
    //! Device Type
    property int        type:           AppSpec.DeviceType.DT_Unknown

    //! Requested Temperature (Cel)
    property real       requestedTemp:  0.0

    //! Current Temperature (Cel)
    property real       currentTemp:    0.0

    //! Requested Humidity (Percent)
    property real       requestedHum:   0.0

    //! Current Humidity (Percent)
    property real       currentHum:     0.0

    //! CO2 Sensor (Air Quality)
    property real       co2:            0.0

    //! TOF: Time of flight (distance sensor)
    property real       tof:            0.0

    //! Backlight
    property Backlight  backlight:      Backlight {}

    //! Wifi
    property Wifi       wifi:           Wifi {}

    //! Fan
    property Fan        fan:            Fan {}

    //! Schedule model
    property Schedule   schedule:       Schedule {}

    /* Functions
     * ****************************************************************************************/
}
