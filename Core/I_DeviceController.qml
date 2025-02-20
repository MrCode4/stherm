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

    readonly property int        temperatureUnit: device.setting?.tempratureUnit ?? AppSpec.defaultTemperatureUnit

    //! Actual values of minimum and maximum temperatures based on temperature unit to use in UI
    property real               _minimumTemperatureUI: getMinValue(device.systemSetup.systemMode, temperatureUnit)
    property real               _maximumTemperatureUI: getMaxValue(device.systemSetup.systemMode, temperatureUnit)

    /* Object Properties
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/

    //! Check device internet and return the proper error.
    function deviceInternetError() : string {
        if (!NetworkInterface.connectedWifi) {
            return AppSpec.noWIFIErrorString

        } else if (!NetworkInterface.hasInternet) {
            return AppSpec.noInternetErrorString;
        }

        return "";
    }

    function updateBacklight(isOn, hue, brightness, shadeIndex) {}

    //! These methods should be overriden by subclasses to provide implementation

    function updateDeviceBacklight(isOn, color):bool {}

    function updateFan(mode: int, workingPerHour: int) {}

    function setVacation(temp_min, temp_max, hum_min, hum_max) {}

    function setSystemModeTo(systemMode: int, force = false, dualFuelManualHeating = false, save = true) {}

    function setVacationOn(on: bool) {}

    function setSettings(brightness, volume, temperatureUnit, adaptive, enabledAlerts, enabledNotifications) {}
    function saveSettings() {}

    function setDesiredTemperature(temperature: real) {}

    function setRequestedHumidity(hum: real) {}

    function setSystemRunDelay(delay: int) {}
    function setSystemCoolingOnly(stage: int) {}
    function setSystemHeatOnly(stage: int) {}
    function setSystemTraditional(coolStage: int, heatStage: int) {}
    function setSystemDualFuelHeating(emergency: bool, stage: int, obState: int, dualFuelThreshold: real) {}
    function setSystemAccessories(accType: int, wireType: int) {}

    function updateInformation() {}

    function updateHold(isHold) {}


    function testRelays(relays) {}

    function setTestData(temperature, on) {}

    function setActivatedSchedule(schedule: ScheduleCPP) {}

    function updateNightMode(nightMode : int) {}

    //! Lock/unlock the application
    function updateAppLockState(isLock : bool, pin: string) : bool {return false;}

    function getMinValue(systemMode, temperatureUnit) {

        var minimumTemperature = AppSpec.autoMinimumTemperatureF;

        switch(systemMode) {
        case AppSpec.EmergencyHeat:
        case AppSpec.Heating: {
            minimumTemperature = AppSpec.minimumHeatingTemperatiureF;
        } break;

        case AppSpec.Cooling: {
            minimumTemperature = AppSpec.minimumCoolingTemperatiureF;
        } break;

        case AppSpec.Auto: {
            minimumTemperature = AppSpec.autoMinimumTemperatureF;
        } break;

        default:
            break;
        }

        var min = (temperatureUnit === AppSpec.TempratureUnit.Fah) ?
                    minimumTemperature : Math.floor(Utils.fahrenheitToCelsius(minimumTemperature))

        return min;
    }


    function getMaxValue(systemMode, temperatureUnit) {
        var maximumTemperature = AppSpec.autoMaximumTemperatureF;

        switch(systemMode) {
        case AppSpec.EmergencyHeat:
        case AppSpec.Heating: {
            maximumTemperature = AppSpec.maximumHeatingTemperatiureF;
        } break;

        case AppSpec.Cooling: {
            maximumTemperature = AppSpec.maximumCoolingTemperatiureF;
        } break;

        case AppSpec.Auto: {
            maximumTemperature = AppSpec.autoMaximumTemperatureF;
        } break;

        default:
            break;
        }

        var max = (temperatureUnit === AppSpec.TempratureUnit.Fah) ?
                    maximumTemperature : Math.floor(Utils.fahrenheitToCelsius(maximumTemperature))

        return max;
    }
}
