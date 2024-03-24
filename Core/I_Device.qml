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
    property int            type:           AppSpec.DeviceType.DT_Unknown

    //! Requested Temperature (Cel)
    property real           requestedTemp:  22.22 // 72 F

    //! Current Temperature (Cel)
    property real           currentTemp:    18.0

    //! Requested Humidity (Percent)
    property real           requestedHum:   45.0

    //! Current Humidity (Percent)
    property real           currentHum:     0.0

    //! CO2 Sensor (Air Quality)
    property real           co2:            0.0

    property int           _co2_id: co2 < 2.9 ? 0 : co2 > 4 ? 2 : 1

    //! TOF: Time of flight (distance sensor)
    property real           tof:            0.0

    //!‌ Device is in hold state or not
    property bool           _isHold:         false

    //! List of all the Messages
    //! List <Message>
    property var            messages:       []

    //! List of device sensors previously paired
    //! List <Sensor>
    property var            sensors:        []

    //! List of device sensors connected
    //! List <Sensor>
    property var            _sensors:        []

    //! List of device schedules
    //! List <Schedule>
    property var            schedules:      []

    //! Backlight
    property Backlight      backlight:      Backlight {}

    //! Fan
    property Fan            fan:            Fan {}

    //! Wiring
    property Wiring         wiring:         Wiring {}

    //! Setting
    property Setting        setting:        Setting {}

    //! Vacation
    property Vacation       vacation:       Vacation {}

    //! Night Mode
    property NightMode       nightMode:      NightMode {}

    // System setup
    property SystemSetup    systemSetup:    SystemSetup {
        _qsRepo: appModel._qsRepo
    }

    //! Contact Contractor
    property ContactContractor contactContractor: ContactContractor {
        _qsRepo: appModel._qsRepo
    }

    /* Functions
     * ****************************************************************************************/
}
