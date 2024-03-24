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

    property var uiSession

    //! Timer to check and run the night mode.
    property Timer nightModeControllerTimer: Timer {
        repeat: true
        running: device.nightMode.mode === AppSpec.NMOn

        interval: 1000

        onTriggered: {
            var currentTime = new Date();
            device.nightMode._running = currentTime.getHours() >= 22 || currentTime.getHours() < 7;
        }
    }

    //! Manage the night mode
    property Connections nightModeController: Connections {
        target: device.nightMode

        function onModeChanged() {
            if (device.nightMode.mode === AppSpec.NMOff) {
                device.nightMode._running = false;
            }
        }

        function on_RunningChanged() {
            if (device.nightMode._running) {
                // Apply night mode
                // Set night mode settings
                // LCD should be set to minimum brightness, and ideally disabled.
                var send_data = [5, 0, device.setting.tempratureUnit,
                                 device.setting.timeFormat, false, false];
                if (!deviceControllerCPP.setSettings(send_data)){
                    console.warn("setting failed");
                }

                // Set night mode backlight.
                // LED light ring will be completely disabled.
                updateDeviceBacklight(false, Qt.color("black"));

                deviceControllerCPP.nightModeControl(true);

            } else {
                // revert to model
                if (device)
                    setSettings(device.setting.brightness, device.setting.volume, device.setting.tempratureUnit,
                                device.setting.timeFormat, false, device.setting.adaptiveBrightness)

                var backlight = device.backlight;
                if (backlight) {
                    var color = device.backlight.backlightFinalColor(backlight.shadeIndex,
                                                                     backlight.hue,
                                                                     backlight.value);
                    updateDeviceBacklight(backlight.on, color);
                }

                deviceControllerCPP.nightModeControl(false);
            }
        }
    }

    property Connections nightMode_BacklightController: Connections {
        target: device.backlight

        enabled: deviceControllerCPP.system.testMode || uiSession.uiTetsMode

        function onOnChanged() {
           updateNightModeWithBacklight();
        }
    }

    property Connections  deviceControllerConnection: Connections {
        target: deviceControllerCPP

        function onAlert(alertLevel : int,
                         alertType : int,
                         alertMessage : string) {

            console.log("Alert: ", alertLevel, alertType, alertMessage);

        }

        function onContractorInfoUpdated(brandName, phoneNumber, iconUrl, url,  techUrl) {

            console.log("onContractorInfoUpdated: ", brandName, phoneNumber, iconUrl, url, techUrl);

            root.device.contactContractor.brandName     = brandName
            root.device.contactContractor.phoneNumber   = phoneNumber
            root.device.contactContractor.iconSource    = iconUrl === "" ? getFromBrandName(brandName): iconUrl
            root.device.contactContractor.qrURL         = url
//            root.device.contactContractor.technicianURL = techUrl
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

        console.log("Update night mode with Backlight")
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

    function updateBacklight(isOn, hue, brightness, shadeIndex)
    {
        var color = device.backlight.backlightFinalColor(shadeIndex, hue, brightness);

        if (updateDeviceBacklight(isOn, color)) {
            device.backlight.on = isOn;
            device.backlight.hue = hue;
            device.backlight.value = brightness;
            device.backlight.shadeIndex = shadeIndex;

            updateNightModeWithBacklight();

        } else {
            console.log("revert the backlight in model: ")
        }

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

    function getFromBrandName(brandName) {
        var name = brandName.toLowerCase();
        if (name === "nuve")
            return "qrc:/Stherm/Images/nuve-icon.png"
        else if (name === "nexgen")
            return "qrc:/Stherm/Images/nexgen.png"
        else if (name === "medley")
            return "qrc:/Stherm/Images/medley.png"
        else if (name === "lees")
            return "qrc:/Stherm/Images/lee_s.png"

        return "qrc:/Stherm/Images/nuve-icon.png"
    }

    function updateNightMode(nightMode : int) {
        if (device)
            device.nightMode.mode = nightMode;
    }

    function updateNightModeWithBacklight() {
        if (deviceControllerCPP.system.testMode || uiSession.uiTetsMode) {
            if (device && device.backlight.on) {
                updateNightMode(AppSpec.NMOff);
            } else {
                updateNightMode(AppSpec.NMOn);
            }
        }
    }
}
