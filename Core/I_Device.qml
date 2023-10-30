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
    property real       requestedTemp:  18.0

    //! Current Temperature (Cel)
    property real       currentTemp:    18.0

    //! Requested Humidity (Percent)
    property real       requestedHum:   0.0

    //! Current Humidity (Percent)
    property real       currentHum:     0.0

    //! CO2 Sensor (Air Quality)
    property real       co2:            0.0

    //! TOF: Time of flight (distance sensor)
    property real       tof:            0.0

    //! System mode
    property int        systemMode:     AppSpec.SystemMode.Auto

    //! List of all the Messages
    //! List <Message>
    property var        messages: []

    //! List of device sensors
    //! List <Sensor>
    property var        sensors: []

    //! Backlight
    property Backlight        backlight:      Backlight {}

    //! Fan
    property Fan              fan:            Fan {}

    //! Schedule model
    property SchedulesModel   schedulesModel: SchedulesModel {
        _qsRepo: appModel._qsRepo
    }

    //! Wiring
    property Wiring     wiring:         Wiring {}

    /* Functions
     * ****************************************************************************************/
}
