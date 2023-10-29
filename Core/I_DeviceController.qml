import QtQuick

/*! ***********************************************************************************************
 * Device Controller
 * ************************************************************************************************/
Item {

    /* Property Declarations
     * ****************************************************************************************/
    required property I_Device device


    /* Object Properties
     * ****************************************************************************************/

    Connections {
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
    //! These methods should be overriden by subclasses to provide implementation

    function updateBacklight(h, s, v) {}

    function updateFan() {}

    function setVacation(temp_min, temp_max, hum_min, hum_max) {}

    function setSystemModeTo(systemMode: int) {}

    function setSettings(brightness, volume, temperature, time, reset, adaptive) {}
}
