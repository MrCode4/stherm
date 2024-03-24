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

    /* Object Properties
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/

    function updateBacklight(isOn, hue, brightness, shadeIndex)
    {
        var color = device.backlight.backlightFinalColor(shadeIndex, hue, brightness);

        if (updateDeviceBacklight(isOn, color))
        {
            device.backlight.on = isOn;
            device.backlight.hue = hue;
            device.backlight.value = brightness;
            device.backlight.shadeIndex = shadeIndex;
        } else {
            console.log("revert the backlight in model: ")
        }

    }

    //! These methods should be overriden by subclasses to provide implementation

    function updateDeviceBacklight(isOn, color):bool {}

    function updateFan(mode: int, workingPerHour: int) {}

    function setVacation(temp_min, temp_max, hum_min, hum_max) {}

    function setSystemModeTo(systemMode: int) {}

    function setVacationOn(on: bool) {}

    function setSettings(brightness, volume, temperatureUnit, timeFormat, reset, adaptive) {}
    function finalizeSettings() {}

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

}
