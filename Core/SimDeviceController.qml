import QtQuick
import Stherm

/*! ***********************************************************************************************
 * Device Controller
 *
 * ************************************************************************************************/
I_DeviceController {

    /* Property Declarations
     * ****************************************************************************************/
    //! Determines whether temprature should increase or decrease
    property int    _changeDirection: 1

    //! Maximum temprature error in simulation
    property real   _currentTempError:  Math.max(1, Math.min(2, (device?.requestedTemp ?? 0) * 0.1))


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
            //! Move simulation temprature to desired one
            if (_changeDirection > 0) {
                device.currentTemp += Math.random() * 1

                if (device.currentTemp > (device.requestedTemp + _currentTempError)) {
                    _changeDirection = -1 //! Make it decrease
                }
            } else {
                device.currentTemp -= Math.random() * 1

                if (device.currentTemp < (device.requestedTemp - _currentTempError)) {
                    _changeDirection = 1 //! Make it increase
                }
            }


//            device.currentTemp = Math.random() * 3 + 20;
//            device.currentHum = Math.random() * 5 + 60
//            device.co2 = Math.random() * 5 + 60
//            device.tof = Math.random() * 5 + 60
        }
    }

}
