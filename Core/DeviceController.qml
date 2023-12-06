import QtQuick

import Stherm

/*! ***********************************************************************************************
 * Device Controller
 *
 * ************************************************************************************************/
I_DeviceController {

    /* Property Declarations
     * ****************************************************************************************/

    property Connections  deviceControllerConnection: Connections {
        target: deviceControllerCPP

        function onAlert(alertLevel : int,
                         alertType : int,
                         alertMessage : string) {

            console.log("Alert: ", alertLevel, alertType, alertMessage);

        }
    }

    //! Connection to send requested temperature and humidity to ui
    property Connections deviceConnection: Connections {
        target: device

        function onRequestedTempChanged() {
            sendRequestedTemperature()
        }

        function onRequestedHumChanged() {
           sendRequestedHumidity();
        }
    }

    /* Object Properties
     * ****************************************************************************************/
    deviceControllerCPP: DeviceControllerCPP {
        systemSetup: device.systemSetup
    }

    /* Signals
     * ****************************************************************************************/

    //! Emit when need to connect to device.
    signal startDeviceRequested();

    //! Emit when need to stop device.
    signal stopDeviceRequested();

    onStartDeviceRequested: {
        console.log("************** Initialize and create connections **************")
        deviceControllerCPP.startDevice();

        console.log("************** set the backlight on initialization **************")
        updateDeviceBacklight(device.backlight.on, device.backlight._color);
    }

    onStopDeviceRequested: {
        deviceControllerCPP.stopDevice();
    }

    Component.onCompleted: {
        sendRequestedTemperature();
        sendRequestedHumidity();
    }

    /* Children
     * ****************************************************************************************/

    /* Methods
     * ****************************************************************************************/

    //! Send Requested temperature to controller
    function sendRequestedTemperature()
    {
        console.log("* requestedTemp: ", device.requestedTemp);
        deviceControllerCPP.setRequestedTemperature(device.requestedTemp);
    }

    //! Send Requested humidity to controller
    function sendRequestedHumidity()
    {
        console.log("* requestedHum: ", device.requestedHum);
        deviceControllerCPP.setRequestedHumidity(device.requestedHum);
    }


    function sendReceive(className, method, data)
    {
        var data_msg = '{"request": {"class": "' + className + '", "method": "' + method + '", "params": ' + JSON.stringify(data) + '}}';

        return deviceControllerCPP.sendRequest(className, method, data)
    }

    function updateDeviceBacklight(isOn, color) : bool
    {
        console.log("starting updateBacklight, color: ", color)

        //! Use a REST request to update device backlight
        var r = Math.round(color.r * 255)
        var g = Math.round(color.g * 255)
        var b = Math.round(color.b * 255)

//        console.log("colors: ", r, ",", g, ",", b)

        //! RGB colors are also sent, maybe device preserve RGB color in off state too.
        //! 1: mode
        //!    LED_STABLE = 0,
        //!    LED_FADE   = 1,
        //!    LED_BLINK  = 2,
        //!    LED_NO_MODE= 3
        var send_data = [r, g, b, 0, isOn ? "true" : "false"];

//        console.log("send data: ", send_data)

        return deviceControllerCPP.setBacklight(send_data);
    }

    function updateFan(mode: int, workingPerHour: int)
    {
        console.log("starting rest for updateFan :", workingPerHour)
        sendReceive('system', 'setFan', workingPerHour);

        // Updatew model
        device.fan.mode = mode
        device.fan.workingPerHour = workingPerHour
    }

    function setVacation(temp_min, temp_max, hum_min, hum_max)
    {
        if (!device)
            return;

        // sendReceive('system', 'setVacation', [temp_min, temp_max, hum_min, hum_max]);

        device.vacation.temp_min = temp_min;
        device.vacation.temp_max = temp_max;
        device.vacation.hum_min  = hum_min;
        device.vacation.hum_max  = hum_max ;

        deviceControllerCPP.setVacation(temp_min, temp_max, hum_min, hum_max);

    }

    function setSystemModeTo(systemMode: int)
    {
        if (systemMode >= 0 && systemMode <= AppSpecCPP.Off) {
            //! Do required actions if any
            sendReceive('system', 'setMode', [ systemMode ]);

            device.systemSetup.systemMode = systemMode;
        }
    }

    //! Set device settings
    function setSettings(brightness, volume, temperatureUnit, timeFormat, reset, adaptive)
    {
        if (!device)
            return;

        console.log("Change settings to : ",
                    "brightness: ",     brightness,     "\n    ",
                    "volume: ",         volume,         "\n    ",
                    "temperature: ",    temperatureUnit,    "\n    ",
                    "timeFormat: ",     timeFormat,           "\n    ",
                    "reset: ",          reset,          "\n    ",
                    "adaptive: ",       adaptive,       "\n    "
                    );

        var send_data = [brightness, volume, temperatureUnit, timeFormat, reset, adaptive];
       if (!deviceControllerCPP.setSettings(send_data)){
           console.warn("setting failed");
           return;
       }

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
        sendReceive('system', 'setTemperature', [temperature]);

        // Update device temperature when setTemperature is successful.
        device.requestedTemp = temperature;
    }

    //! Read data from system with getMainData method.
    function updateInformation()
    {
//        console.log("--------------- Start: updateInformation -------------------")
        var result = deviceControllerCPP.getMainData();

        // should be catched later here
        device.currentHum = result?.humidity ?? 0
        device.currentTemp = result?.temperature ?? 0
        device.co2 = result?.co2 ?? 0
        //        device.setting.brightness = result?.brighness ?? 0

        //        device.fan.mode?

        //        console.log("--------------- End: updateInformation -------------------")
    }

    function updateHold(isHold)
    {
        // should be updated to inform the logics
        var result = sendReceive('system', 'setHold', [isHold]);

        device.isHold = isHold;
    }
}
