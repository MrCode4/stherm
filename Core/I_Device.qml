import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * I_Device: keeps the main data of app
 * NOTE: Avoid adding '_' prefixed object properties (e.g. property Lock _lock: Lock {}) to the
 * I_device object and its descendants.
 * Introduce new object properties (e.g. Wiring, Setting, Lock, ...) exclusively within the
 * I_device base object.
 * ************************************************************************************************/
QSObject {
    id: appModel

    /* Property Declarations
     * ****************************************************************************************/
    //! Device Type
    property int            type:           AppSpec.DeviceType.DT_Unknown

    //! Requested Temperature (Cel)
    property real           requestedTemp:  22.22 // 72 F

    //! Requested Min temperature in auto mode (Cel)
    property real           autoMinReqTemp: 21.1111 // 70 F

    //! Requested Max temperature in auto mode (Cel)
    property real           autoMaxReqTemp: 23.3333 // 74 F

    //! Current Temperature (Cel)
    property real           currentTemp:    18.0

    //! Requested Humidity (Percent)
    property real           requestedHum:   45.0

    //! Current Humidity (Percent)
    property real           currentHum:     0.0

    //! CO2 Sensor (Air Quality)
    property real           co2:            0.0

    property int           _co2_id: airQuality(co2)

    //! TOF: Time of flight (distance sensor)
    property real           tof:            0.0

    //!‌ Device is in hold state or not
    property bool          isHold:         false

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

    //! Private policy and terms of use
    property UserPolicyTerms userPolicyTerms: UserPolicyTerms {}

    //! Lock model
    property Lock            _lock: Lock {}

    /* Functions
     * ****************************************************************************************/

    //! Air quality
    function airQuality(co2Value : int) : int {
        return co2Value < 2.9 ? 0 : co2Value > 4 ? 2 : 1;
    }
}
