import QtQuick

import Stherm

/*! ***********************************************************************************************
 * Device Controller
 *
 * ************************************************************************************************/
I_DeviceController {
    id: root

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

    property Connections networkInterface: Connections {
        target: NetworkInterface

        function onHasInternetChanged() {
            deviceControllerCPP.system.wifiConnected(NetworkInterface.hasInternet);
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
        //! initialize the device and config
        // as well as device io which may TODO refactor later and call it on demand
        deviceControllerCPP.startDevice();

        // TODO    we might call this contitionally
        console.log("************** set the backlight on initialization **************")
        updateDeviceBacklight(device.backlight.on, device.backlight._color);

        var send_data = [device.setting.brightness, device.setting.volume, device.setting.tempratureUnit, device.setting.timeFormat, false, device.setting.adaptiveBrightness];
       if (!deviceControllerCPP.setSettings(send_data)){
           console.warn("setting failed");
       }
    }

    onStopDeviceRequested: {
        deviceControllerCPP.stopDevice();
    }

    Component.onCompleted: {
        console.log("* requestedTemp initial: ", device.requestedTemp);
        console.log("* requestedHum initial: ", device.requestedHum);
        deviceControllerCPP.setRequestedTemperature(device.requestedTemp);
        deviceControllerCPP.setRequestedHumidity(device.requestedHum);
        // TODO what parameters should be initialized here?
        deviceControllerCPP?.setFan(device.fan.mode, device.fan.workingPerHour)
    }

    /* Children
     * ****************************************************************************************/

    /* Methods
     * ****************************************************************************************/

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
        //! TODo required actions if any


        if (deviceControllerCPP?.setFan(mode, workingPerHour) ?? false) {
            // Update model
            device.fan.mode = mode;
            device.fan.workingPerHour = workingPerHour;
        }
    }

    function setVacation(temp_min, temp_max, hum_min, hum_max)
    {
        if (!device)
            return;

        deviceControllerCPP.setVacation(temp_min, temp_max, hum_min, hum_max);

        device.vacation.temp_min = temp_min;
        device.vacation.temp_max = temp_max;
        device.vacation.hum_min  = hum_min;
        device.vacation.hum_max  = hum_max ;


    }

    function setSystemModeTo(systemMode: int)
    {
        if (systemMode === AppSpecCPP.Vacation) {
            setVacationOn(true);

        } else if (systemMode >= 0 && systemMode <= AppSpecCPP.Off) {
            //! TODo required actions if any

            device.systemSetup.systemMode = systemMode;
        }
    }

    //! On/off the vacation.
    function setVacationOn(on: bool) {
        device.systemSetup.isVacation = on;
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

        // Update setting when setSettings is successful.
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
        //! Apply temperature in backend
        deviceControllerCPP.setRequestedTemperature(temperature);

        // Update device temperature when setTemperature is successful.
        device.requestedTemp = temperature;
    }

    function setRequestedHumidity(humidity: real) {
        //! Apply humidity in backend
        deviceControllerCPP.setRequestedHumidity(humidity);

        //! Update requested humidity to device
        device.requestedHum = humidity;
    }

    function setSystemRunDelay(delay: int) {
        device.systemSetup.systemRunDelay = delay
    }

    function setSystemCoolingOnly(stage: int) {
        device.systemSetup.coolStage  = stage;
        device.systemSetup.systemType = AppSpecCPP.CoolingOnly;

        if (device.systemSetup.systemMode !== AppSpecCPP.Off && device.systemSetup.systemMode !== Vacation)  {
            setSystemModeTo(AppSpecCPP.Cooling)
        }
    }

    function setSystemHeatOnly(stage: int) {
        device.systemSetup.heatStage  = stage;
        device.systemSetup.systemType = AppSpecCPP.HeatingOnly;
        if (device.systemSetup.systemMode !== AppSpecCPP.Off && device.systemSetup.systemMode !== Vacation)  {
            setSystemModeTo(AppSpecCPP.Heating)
        }
    }

    function setSystemHeatPump(emergency: bool, stage: int, obState: int) {
        device.systemSetup.heatPumpEmergency = emergency;
        device.systemSetup.heatStage = stage;
        device.systemSetup.coolStage = stage;
        device.systemSetup.heatPumpOBState = obState;
        device.systemSetup.systemType = AppSpecCPP.HeatPump;
    }

    function setSystemTraditional(coolStage: int, heatStage: int) {
        device.systemSetup.coolStage = coolStage;
        device.systemSetup.heatStage = heatStage;
        device.systemSetup.systemType = AppSpecCPP.Conventional;
    }

    //! Read data from system with getMainData method.
    function updateInformation()
    {
//        console.log("--------------- Start: updateInformation -------------------")
        var result = deviceControllerCPP.getMainData();

        // should be catched later here
        device.currentHum = result?.humidity ?? 0
        device.currentTemp = result?.temperature ?? 0
        device.co2 = result?.iaq ?? 0 // use iaq as indicator for air quality
        //        device.setting.brightness = result?.brighness ?? 0

        //        device.fan.mode?

        //        console.log("--------------- End: updateInformation -------------------")
    }

    function updateHold(isHold)
    {
        // TODO should be updated to inform the logics

        device._isHold = isHold;
    }

    function setSystemAccesseories(accType: int, wireType: int) {
        device.systemSetup.systemAccessories.setSystemAccessories(accType, wireType);
    }
    
    function testRelays(relays) {
        var send_data = [relays.isR, relays.isC, relays.isG, relays.isY1, relays.isY2, relays.isT2,
                         relays.isW1, relays.isW2, relays.isW3, relays.isOB, relays.isT1p, relays.isT1n];
        deviceControllerCPP.setTestRelays(send_data);
    }


    function setTestData(temperature, on) {
        var send_data = {
            "temperature": temperature,
        }
        deviceControllerCPP.setOverrideMainData(on ? send_data : {})
    }

    function getTestData() {
        return deviceControllerCPP.getMainData();
    }

    function setActivatedSchedule(schedule: ScheduleCPP) {

        if (root.currentSchedule === schedule)
            return;

        root.currentSchedule = schedule;
        deviceControllerCPP.setActivatedSchedule(schedule);
    }
}
