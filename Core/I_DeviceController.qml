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

    //! Minimum value for temperature model (Fah)
    property real               _minimumTemperatureF: 40

    //! Maximum value for temperature model (Fah)
    property real               _maximumTemperatureF: 90

    //! Minimum value for temperature slider (Fah)
    //! - Has schedule effect
    property real               _minimumTemperatureUIF: {
        if (currentSchedule) {
            return AppSpec.minimumTemperatureF;
        }

        return _minimumTemperatureF;
    }

    //! Maximum value for temperature slider
    //! - Has schedule effect
    property real               _maximumTemperatureUIF: {
        if (currentSchedule) {
            return AppSpec.maximumTemperatureF;
        }

        return _maximumTemperatureF;
    }

    //! Connection
    property Connections systemSetupConnection: Connections {
        target: device.systemSetup

        function onSystemModeChanged() {
            var minimumTemperature = _minimumTemperatureF;
            var maximumTemperature = _maximumTemperatureF;

            switch(device.systemSetup.systemMode) {
            case AppSpec.Heating: {
                minimumTemperature = 40;
                maximumTemperature = 85;
            } break;

            case AppSpec.Cooling: {
                minimumTemperature = 60;
                maximumTemperature = 90;
            } break;

            case AppSpec.Auto: {
                minimumTemperature = AppSpec.autoMinimumTemperatureF;
                maximumTemperature = AppSpec.autoMaximumTemperatureF;
            } break;

            default:
                break;
            }

            _minimumTemperatureF = minimumTemperature;
            _maximumTemperatureF = maximumTemperature;

            // mode can change minimum and maximum range of desired temperature so need to clamp desired temperature with mode changes.
            var clampTemperature = Utils.clampValue(device.requestedTemp, _minimumTemperatureC, _maximumTemperatureC);

            if (clampTemperature !== device.requestedTemp) {
                setDesiredTemperature(clampTemperature);
            }
        }
    }


    //! Maximum and Minimum slider temperature in Celsius
    property real               _minimumTemperatureC: Math.floor(Utils.fahrenheitToCelsius(_minimumTemperatureF))
    property real               _maximumTemperatureC: Math.floor(Utils.fahrenheitToCelsius(_maximumTemperatureF))


    //! Actual values of minimum and maximum temperatures based on temperature unit
    property real               _minimumTemperature:  device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                      ? _minimumTemperatureF : _minimumTemperatureC

    property real               _maximumTemperature:  device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                      ? _maximumTemperatureF : _maximumTemperatureC

    //! Maximum and Minimum model temperature in Celsius to use in UI
    property real               _minimumTemperatureUIC: Math.floor(Utils.fahrenheitToCelsius(_minimumTemperatureUIF))
    property real               _maximumTemperatureUIC: Math.floor(Utils.fahrenheitToCelsius(_maximumTemperatureUIF))

    //! Actual values of minimum and maximum temperatures based on temperature unit to use in UI
    property real               _minimumTemperatureUI:  device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                        ? _minimumTemperatureUIF : _minimumTemperatureUIC

    property real               _maximumTemperatureUI:  device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                        ? _maximumTemperatureUIF : _maximumTemperatureUIC

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

    function setSettings(brightness, volume, temperatureUnit, adaptive, enabledAlerts, enabledNotifications) {}
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

    //! Lock/unlock the application
    function lock(isLock : bool, pin: string) : bool {return false;}

}
