import QtQuick

import Stherm
/*! ***********************************************************************************************
 * Device Controller
 * ************************************************************************************************/
QtObject {

    /* Property Declarations
     * ****************************************************************************************/
    required property I_Device device

    //! Holds current Schedule
    property ScheduleCPP       currentSchedule: null

    property DeviceControllerCPP deviceControllerCPP: null

    property int                 startMode: -1

    //! Minimum value for temperature slider (Fah)
    property real               _minimumTemperatureF: {
        switch(device.systemSetup.systemMode) {
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
        switch(device.systemSetup.systemMode) {
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
    property real               _maximumTemperatureC: Math.floor(Utils.fahrenheitToCelsius(_maximumTemperatureF))

    //! Actual values of minimum and maximum temperatures based on temperature unit
    property real               _minimumTemperature:  device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                      ? _minimumTemperatureF : _minimumTemperatureC

    property real               _maximumTemperature:  device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                      ? _maximumTemperatureF : _maximumTemperatureC

    /* Object Properties
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/

    function updateBacklight(isOn, hue, brightness, shadeIndex) {}

    //! These methods should be overriden by subclasses to provide implementation

    function updateDeviceBacklight(isOn, color):bool {}

    function updateFan(mode: int, workingPerHour: int) {}

    function setVacation(temp_min, temp_max, hum_min, hum_max) {}

    function setSystemModeTo(systemMode: int) {}

    function setVacationOn(on: bool) {}

    function setSettings(brightness, volume, temperatureUnit, adaptive) {}
    function pushSettings() {}

    function setDesiredTemperature(temperature: real) {}

    function setRequestedHumidity(hum: real) {}

    function setSystemRunDelay(delay: int) {}
    function setSystemCoolingOnly(stage: int) {}
    function setSystemHeatOnly(stage: int) {}
    function setSystemHeatPump(emergency: bool, stage: int, obState: int) {}
    function setSystemTraditional(coolStage: int, heatStage: int) {}
    function setSystemAccesseories(accType: int, wireType: int) {}

    function updateInformation() {}

    function updateHold(isHold) {}


    function testRelays(relays) {}

    function setTestData(temperature, on) {}

    function setActivatedSchedule(schedule: ScheduleCPP) {}

    function updateNightMode(nightMode : int) {}

}
