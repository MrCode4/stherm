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

    function updateBacklight() {}

    function updateFan() {}

    function setSystemModeTo(systemMode: int) {}
}
