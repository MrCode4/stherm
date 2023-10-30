import QtQuick

/*! ***********************************************************************************************
 * Device Controller
 * ************************************************************************************************/
QtObject {

    /* Property Declarations
     * ****************************************************************************************/
    required property I_Device device


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

    function updateBacklight(h, s, v)
    {
        device.backlight.hue = h;
        device.backlight.saturation = s;
        device.backlight.value = v;

        updateDeviceBacklight();
    }

    //! These methods should be overriden by subclasses to provide implementation

    function updateDeviceBacklight() {}

    function updateFan() {}

    function setVacation(temp_min, temp_max, hum_min, hum_max) {}

    function setSystemModeTo(systemMode: int) {}

    function setSettings(brightness, volume, temperature, time, reset, adaptive) {}

    function setDesiredTemperature(temperature: real) {}
}
