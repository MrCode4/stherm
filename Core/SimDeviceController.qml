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
    property int    _tempChangeDirection: 1

    //! Maximum temprature error in simulation
    property real   _currentTempError:  Math.max(1, Math.min(2, (device?.requestedTemp ?? 0) * 0.1))

    //! Determines whether humidity should increase or decrease
    property int    _humChangeDirection: 1

    //! Maximum humidity error in simulation
    property real   _currentHumError:  Math.max(1, Math.min(3, (device?.requestedHum ?? 0) * 0.1))


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
            if (_tempChangeDirection > 0) {
                device.currentTemp += Math.random() * 3

                if (device.currentTemp > (device.requestedTemp + _currentTempError)) {
                    _tempChangeDirection = -1 //! Make it decrease
                }
            } else {
                device.currentTemp -= Math.random() * 3

                if (device.currentTemp < (device.requestedTemp - _currentTempError)) {
                    _tempChangeDirection = 1 //! Make it increase
                }
            }

            //! Randomly change humidity towards requested one
            if (_humChangeDirection > 0) {
                device.currentHum += Math.random() * 3

                if (device.currentHum > (device.requestedHum + _currentHumError)) {
                    _humChangeDirection = -1 //! Make it decrease
                }
            } else {
                device.currentHum -= Math.random() * 3

                if (device.currentHum < (device.requestedHum - _currentHumError)) {
                    _humChangeDirection = 1 //! Make it increase
                }
            }

//            device.co2 = Math.random() * 5 + 60
//            device.tof = Math.random() * 5 + 60
        }
    }

    /* Methods
     * ****************************************************************************************/
    //! Override I_DeviceController's methods
    function updateBacklight(h, s, l)
    {
        //! Change background color of application
        device.backlight.hue = h;
        device.backlight.saturation = s;
        device.backlight.value = l;

        if (device.backlight.on) {
            AppStyle.backgroundColor = device.backlight.color;
        } else {
            AppStyle.backgroundColor = "#000000";
        }
    }

    function setSystemModeTo(systemMode: int)
    {
        if (systemMode >= 0 && systemMode <= I_Device.SystemMode.Off) {
            device.systemMode = systemMode;

            //! Do required actions if any
        }
    }
}
