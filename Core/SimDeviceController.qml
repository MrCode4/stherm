import QtQuick

import Ronia
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
    property Timer _timer: Timer {
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
    function updateDeviceBacklight()
    {
        console.log("settign sim background color")

        Style.background = device.backlight.on ?  device.backlight.color : "#000000";
    }

    function setVacation(temp_min, temp_max, hum_min, hum_max)
    {
        if (!device)
            return

        // udpate model
        device.vacation.temp_min = temp_min;
        device.vacation.temp_max = temp_max;
        device.vacation.hum_min  = hum_min;
        device.vacation.hum_max  = hum_max ;
    }

    function setSystemModeTo(systemMode: int)
    {
        if (systemMode >= 0 && systemMode <= AppSpec.SystemMode.Off) {
            device.systemMode = systemMode;

            //! Do required actions if any
        }
    }

    //! Update fan
    function updateFan(mode: int, workingPerHour: int)
    {
        // Updatew model
        device.fan.mode = mode
        device.fan.workingPerHour = workingPerHour
    }

    //! Set device settings
    function setSettings(brightness, volume, temperatureUnit, timeFormat, reset, adaptive)
    {
        // udpate model
        // Update setting when sendReceive is successful.
        if (device.setting.brightness !== brightness) {
            device.setting.brightness = brightness;
        }

        if (device.setting.volume !== volume) {
            device.setting.volume = volume;
        }

        if (device.setting.adaptiveBrightness !== adaptive) {
            device.setting.adaptiveBrightness = adaptive;
        }

        if (device.setting.timeFormat !== timeFormat) {
            device.setting.timeFormat = timeFormat;
        }

        if (device.setting.tempratureUnit !== temperatureUnit) {
            device.setting.tempratureUnit = temperatureUnit;
        }
    }

    //! Set temperature to device (system) and update model.
    function setDesiredTemperature(temperature: real) {
        device.requestedTemp = temperature;
    }
}
