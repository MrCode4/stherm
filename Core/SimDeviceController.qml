import QtQuick
import Stherm

/*! ***********************************************************************************************
 * Device Controller
 *
 * ************************************************************************************************/
I_DeviceController {

    /* Property Declarations
     * ****************************************************************************************/


    /* Object Properties
     * ****************************************************************************************/


    /* Children
     * ****************************************************************************************/
    //! Read sensor data (simulation)
    //! todo: move this to the interface class
    Timer {
        interval: AppSpec.simReadInterval
        running: true
        repeat: true
        onTriggered: {
            device.currentTemp = Math.random() * 3 + 20;
            device.currentHum = Math.random() * 5 + 60
            device.co2 = Math.random() * 5 + 60
            device.tof = Math.random() * 5 + 60
        }
    }

}
