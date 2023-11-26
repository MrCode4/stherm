import QtQuick

import Stherm
/*! ***********************************************************************************************
 * Device Controller
 * ************************************************************************************************/
QtObject {

    /* Property Declarations
     * ****************************************************************************************/
    required property I_Device device


    property DeviceControllerCPP deviceControllerCPP: null

    /* Object Properties
     * ****************************************************************************************/

    property Connections connections: Connections {
        target: device

        function onRequestedTempChanged() {
            // send request (in this case it's simulation only)
        }

        function onRequestedHumChanged() {
            // send request (in this case it's simulation only)
        }
    }


    /* Functions
     * ****************************************************************************************/

    function updateBacklight(isOn, color, shadeIndex)
    {
        if (updateDeviceBacklight(isOn, color))
        {
            device.backlight.on = isOn;
            device.backlight.hue = color.hsvHue;
            device.backlight.saturation = color.hsvSaturation;
            device.backlight.value = color.hsvValue;
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

    function setSettings(brightness, volume, temperatureUnit, timeFormat, reset, adaptive) {}

    function setDesiredTemperature(temperature: real) {}

    function updateInformation() {}
}
