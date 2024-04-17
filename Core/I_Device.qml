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

    //! Requested Min temperature in auto mode (Cel)
    property real           autoMinReqTemp: 4.44 // 40 F

    //! Requested Max temperature in auto mode (Cel)
    property real           autoMaxReqTemp: 32.22 // 90 F

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

    //! Minimum value for temperature slider (Fah)
    property real               _minimumTemperatureF: {
        switch(systemSetup.systemMode) {
        case AppSpec.Heating:
            return 40;
        case AppSpec.Cooling:
            return 60;
        case AppSpec.Auto:
            return 40;
        default:
            return 64; //! In Off and Schedule mode (doesn't matter)
        }
    }

    //! Maximum value for temperature slider
    property real               _maximumTemperatureF: {
        switch(systemSetup.systemMode) {
        case AppSpec.Heating:
            return 85;
        case AppSpec.Cooling:
            return 90;
        case AppSpec.Auto:
            return 90;
        default:
            return 90; //! In Off and Schedule mode (doesn't matter)
        }
    }

    //! Maximum and Minimum temperature in Celsius
    property real               _minimumTemperatureC: Math.floor(Utils.fahrenheitToCelsius(_minimumTemperatureF))
    property real               _maximumTemperatureC: Math.ceil(Utils.fahrenheitToCelsius(_maximumTemperatureF))

    //! Actual values of minimum and maximum temperatures based on temperature unit
    property real               _minimumTemperature:  setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                      ? _minimumTemperatureF : _minimumTemperatureC

    property real               _maximumTemperature:  setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                      ? _maximumTemperatureF : _maximumTemperatureC

    /* Functions
     * ****************************************************************************************/

    //! Air quality
    function airQuality(co2Value : int) : int {
        return co2Value < 2.9 ? 0 : co2Value > 4 ? 2 : 1;
    }
}
