import QtQuick

import Stherm
import QtQuickStream

/*! ***********************************************************************************************
 * Device Controller
 *
 * ************************************************************************************************/
I_DeviceController {
    id: root

    /* Property Declarations
     * ****************************************************************************************/

    property SchedulesController schedulesController

    property MessageController   messageController

    property int editMode: AppSpec.EMNone

    property bool initalSetup: false;

    property var uiSession

    //! Night mode brighness when screen saver is off.
    property real nightModeBrightness: -1
    property real targetNightModeBrightness: Math.min(50, (device.setting.adaptiveBrightness ? deviceControllerCPP.adaptiveBrightness : device.setting.brightness))

    //! Timer to check and run the night mode.
    property Timer nightModeControllerTimer: Timer {
        repeat: true
        running: device.nightMode.mode === AppSpec.NMOn

        interval: 1000

        onTriggered: {
            var currentTime = new Date();
            device.nightMode._running = currentTime.getHours() >= 21 || currentTime.getHours() < 7;
        }
    }

    //! Manage the night mode
    property Connections nightModeController: Connections {
        target: device.nightMode

        function onModeChanged() {
            if (device.nightMode.mode === AppSpec.NMOff) {
                device.nightMode._running = false;
                brightnessTimer.stop();
            }
        }

        function on_RunningChanged() {
            runNightMode();
        }
    }

    //! Manage the night mode with screen saver
    property Connections nightMode_screenSaverController: Connections {
        target: ScreenSaverManager

        enabled: device.nightMode._running

        function onStateChanged() {
            if (ScreenSaverManager.state !== ScreenSaverManager.Timeout) {
                if (nightModeBrightness !== targetNightModeBrightness) {
                    brightnessTimer.start();
                    nightModeBrightness = targetNightModeBrightness;
                }

                deviceControllerCPP.setCPUGovernor(AppSpec.CPUGondemand);

            } else {
                brightnessTimer.stop();
                setBrightnessInNightMode(5, device.setting.volume, false);
                nightModeBrightness = 5;

                deviceControllerCPP.setCPUGovernor(AppSpec.CPUGpowersave);
            }
        }
    }

    //! The screen will gradually (within up to 3 seconds) set the screen brightness to targetNightModeBrighness
    property Timer brightnessTimer: Timer {

        property int steps: 1

        running: false
        onRunningChanged: {
            steps = 1;
        }

        repeat: true
        interval: Math.round(3000 / Math.abs(targetNightModeBrightness - 5));

        onTriggered: {
            setBrightnessInNightMode(5 + steps, device.setting.volume, false);
            steps++;
            if (steps > Math.abs(targetNightModeBrightness - 5))
                stop();
        }
    }

    property Connections  deviceControllerConnection: Connections {
        target: deviceControllerCPP

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

    property Connections sync: Connections {
        target: deviceControllerCPP.system

        function onSettingsReady(settings) {
            if (!deviceControllerCPP.system.canFetchServer || settingsPush.running || settingsPushRetry.running) {
                console.log("We have some changes that not applied on the server.")
                return;
            }

            checkQRurl(settings.qr_url)
            updateHoldServer(settings.hold)
            updateFanServer(settings.fan)
            setSettingsServer(settings.setting)
            setRequestedHumidityFromServer(settings.humidity)
            setDesiredTemperatureFromServer(settings.temp)
            setSystemModeServer(settings.mode_id)
            setSchedulesFromServer(settings.schedule)
            setVacationServer(settings.vacation)
            setMessagesServer(settings.messages)
            checkSensors(settings.sensors)
            setSystemSetupServer(settings.system)

            setAutoTemperatureFromServer(settings);
        }

        function onCanFetchServerChanged() {
            if (deviceControllerCPP.system.canFetchServer) {
                settingsPushRetry.failed = false;
                settingsPushRetry.interval = 5000;
                settingsPush.hasSettings = false
            }
        }

        function onPushFailed() {
            if (settingsPushRetry.failed) {
                settingsPushRetry.interval = settingsPushRetry.interval *2;
                if (settingsPushRetry.interval > 60000)
                    settingsPushRetry.interval = 60000;
            } else {
                settingsPushRetry.failed = true;
            }

            settingsPushRetry.start()
        }
    }

    property Timer  settingsPush: Timer {
        repeat: false;
        running: false;
        interval: 100;

        property bool hasSettings : false

        onTriggered: {
            pushToServer();
        }
    }

    property Timer  settingsPushRetry: Timer {
        repeat: false;
        running: false;
        interval: 5000;

        property bool failed: false

        onTriggered: {
            settingsPush.start();
        }
    }

    property Timer  settingsLoader: Timer {
        repeat: true;
        running: !initalSetup;
        interval: 5000;
        onTriggered:
        {
            if (!deviceControllerCPP.system.fetchSettings()) {
                var intervalNew = interval * 2;
                if (intervalNew > 60000)
                    intervalNew = 60000;
                interval = intervalNew;
                console.log("fetching failed, backing off to ", interval)
            } else {
                interval = 5000;
                console.log("fetching success, back to ", interval)
            }
        }
    }

    property Timer editModeTimer: Timer {
        repeat: false
        running: false
        interval: 15000
        property int disableFlags : AppSpec.EMNone;

        onTriggered: {
            root.editMode = root.editMode & ~disableFlags;
            disableFlags = AppSpec.EMNone;
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

        var send_data = [device.setting.brightness, device.setting.volume,
                         device.setting.tempratureUnit, device.setting.adaptiveBrightness];
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
        deviceControllerCPP?.setFan(device.fan.mode, device.fan.workingPerHour);
        deviceControllerCPP.setAutoMaxReqTemp(device.autoMaxReqTemp);
        deviceControllerCPP.setAutoMinReqTemp(device.autoMinReqTemp);
    }

    /* Children
     * ****************************************************************************************/

    /* Methods
     * ****************************************************************************************/

    function setInitialSetup(init: bool) {
        initalSetup = init;
    }

    function updateEditMode(editMode : int, enable = true) {

        if (enable) {
            root.editMode = root.editMode | editMode; // add flag
            // remove from disabling flags
            editModeTimer.disableFlags = editModeTimer.disableFlags & ~editMode

        } else { // add to current disable flags and restart timer
            editModeTimer.disableFlags = editModeTimer.disableFlags | editMode
            if (editModeTimer.running)
                editModeTimer.stop();
            editModeTimer.start();
        }
    }

    function editModeEnabled(editMode : int) {
        return (root.editMode & editMode) !== 0;
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

        updateNightModeWithBacklight(isOn);
        //        console.log("send data: ", send_data)


        return deviceControllerCPP.setBacklight(send_data);
    }

    function updateFanServer(settings : var) {

        if (editModeEnabled(AppSpec.EMFan)) {
            console.log("The fan page is being edited and cannot be updated by the server.")
            return;
        }

        updateFan(settings.mode, settings.workingPerHour)
    }

    function updateBacklight(isOn, hue, brightness, shadeIndex)
    {
        var color = device.backlight.backlightFinalColor(shadeIndex, hue, brightness);

        if (updateDeviceBacklight(isOn, color)) {
            device.backlight.on = isOn;
            device.backlight.hue = hue;
            device.backlight.value = brightness;
            device.backlight.shadeIndex = shadeIndex;

        } else {
            console.log("revert the backlight in model: ")
        }

    }

    function updateFan(mode: int, workingPerHour: int)
    {
        if (device.fan.mode === mode && device.fan.workingPerHour === workingPerHour)
            return;

        if (deviceControllerCPP?.setFan(mode, workingPerHour) ?? false) {
            // Update model
            device.fan.mode = mode;
            device.fan.workingPerHour = workingPerHour;
        }
    }

    function setVacationServer(settings : var)
    {
        if (editModeEnabled(AppSpec.EMVacation)) {
            console.log("The vacation is being edited and cannot be updated by the server.")
            return;
        }

        setVacation(settings.min_temp, settings.max_temp, settings.min_humidity, settings.max_humidity)
        setVacationOnFromServer(settings.is_enable)
    }

    function setVacation(temp_min, temp_max, hum_min, hum_max)
    {
        if (!device)
            return;

        if (device.vacation.temp_min === temp_min &&
            device.vacation.temp_max === temp_max &&
            device.vacation.hum_min  === hum_min &&
            device.vacation.hum_max  === hum_max) {
            return;
        }

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
            pushSettings();
        }
    }

    //! On/off the vacation from server.
    function setVacationOnFromServer(on: bool) {
        device.systemSetup.isVacation = on;
    }

    //! On/off the vacation.
    function setVacationOn(on: bool) {
        device.systemSetup.isVacation = on;

        pushSettings();
    }

    //! Set time format
    function setTimeFormat(timeFormat : int) {
        if (device.setting.timeFormat !== timeFormat) {
            device.setting.timeFormat = timeFormat;
            return true;
        }

        return false;
    }

    //! Set device settings
    function setSettings(brightness, volume, temperatureUnit, adaptive)
    {
        if (!device){
            console.log("corrupted device")
            return false;
        }

        var send_data = [brightness, volume, temperatureUnit, adaptive];
        var current_data = [device.setting.brightness, device.setting.volume,
                            device.setting.tempratureUnit, device.setting.adaptiveBrightness]
        if (send_data.toString() === current_data.toString()) {
            console.log("ignored setings")
            return false;
        }

        if (!device.nightMode._running && !deviceControllerCPP.setSettings(send_data)){
            console.warn("setting failed");
            return false;
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

        if (device.setting.tempratureUnit !== temperatureUnit) {
            device.setting.tempratureUnit = temperatureUnit;
        }

        return true;
    }

    function pushUpdateToServer(settings: bool){
        if (settings)
            settingsPush.hasSettings = true

        if (settingsPush.running)
            return;

        if (settingsPushRetry.running)
            settingsPushRetry.stop();
        else if (settingsPushRetry.failed)
            return;

        settingsPush.start()
    }

    function pushSettings() {
        pushUpdateToServer(true);

        if (uiSession)
            AppCore.defaultRepo.saveToFile(uiSession.configFilePath);
    }

    function setSettingsServer(settings: var) {
        if (!editModeEnabled(AppSpec.EMDateTime)) {
            if (device.setting.currentTimezone !== settings.currentTimezone)
                device.setting.currentTimezone = settings.currentTimezone;

            if (device.setting.effectDst !== settings.effectDst)
                device.setting.effectDst = settings.effectDst;

            if (device.setting.timeFormat !== settings.timeFormat)
                device.setting.timeFormat = settings.timeFormat;

        } else {
            console.log("The Date time settings is being edited and cannot be updated by the server.")
        }

        if (!editModeEnabled(AppSpec.EMSettings)) {
            if (!setSettings(settings.brightness, settings.speaker,
                        settings.temperatureUnit, settings.brightness_mode))
                console.log("The system settings is not applied from server")

        } else {
            console.log("The system settings is being edited and cannot be updated by the server.")
        }
        setBacklightServer(settings.backlight)
    }

    function setBacklightServer(settings: var) {
        if (editModeEnabled(AppSpec.EMBacklight)) {
            console.log("The backlight is being edited and cannot be updated by the server.")
            return;
        }

        if (settings.on === device.backlight.on && settings.hue === device.backlight.hue &&
                settings.value === device.backlight.value && settings.shadeIndex === device.backlight.shadeIndex)
            return;

        updateBacklight(settings.on, settings.hue, settings.value,
                        settings.shadeIndex)
    }

    function pushToServer() {
        var send_data = {
            "temp": device.requestedTemp,
            "humidity": device.requestedHum,
            "current_humidity": device.currentHum.toString(),
            "current_temp": device.currentTemp.toString(),
            "co2_id": device._co2_id + 1,
            "hold" : device.isHold,
            "mode_id" : device.systemSetup.systemMode + 1,
            "auto_temp_high" : device.autoMaxReqTemp,
            "auto_temp_low" : device.autoMinReqTemp,
            "fan" : {
                "mode" : device.fan.mode,
                "workingPerHour": device.fan.workingPerHour,
            },
            "backlight" : {
                "on": device.backlight.on,
                "hue": device.backlight.hue,
                "value": device.backlight.value,
                "shadeIndex": device.backlight.shadeIndex
            },
            "settings" : {
                "brightness": device.setting.brightness,
                "brightness_mode": device.setting.adaptiveBrightness ? 1 : 0,
                "speaker": device.setting.volume,
                "temperatureUnit": device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? 1 : 0,
                "timeFormat": device.setting.timeFormat === AppSpec.TimeFormat.Hour24 ? 1 : 0,
                "currentTimezone": device.setting.currentTimezone.length > 0 ? device.setting.currentTimezone : "UTC",
                "effectDst": device.setting.effectDst,
            },
            "sensors" : [],
            "schedules" : [],
            "messages" : [],
            "vacation" : {
                "min_humidity" : device.vacation.hum_min,
                "max_humidity": device.vacation.hum_max,
                "min_temp": device.vacation.temp_min,
                "max_temp": device.vacation.temp_max,
                "is_enable": device.systemSetup.isVacation ? "t" : "f",
            },
            "system" : {
                "type": AppSpec.systemTypeString(device.systemSetup.systemType),
                "coolStage": device.systemSetup.coolStage,
                "heatStage": device.systemSetup.heatStage,
                "heatPumpOBState": device.systemSetup.heatPumpOBState,
                "heatPumpEmergency": device.systemSetup.heatPumpEmergency,
                "systemRunDelay": device.systemSetup.systemRunDelay,
                "systemAccessories": {
                    "wire": AppSpec.accessoriesWireTypeString(device.systemSetup.systemAccessories.accessoriesWireType),
                    "mode": device.systemSetup.systemAccessories.accessoriesWireType === AppSpec.None ?
                                AppSpec.ATNone : device.systemSetup.systemAccessories.accessoriesType,
                }
            },
        }

        device.schedules.forEach(schedule =>
                                 {
                                     send_data.schedules.push(
                                         {
                                             "is_enable": schedule.enable,
                                             "name": schedule.name,
                                             "type_id": schedule.type,
                                             "start_time": schedule.startTime,
                                             "end_time": schedule.endTime,
                                             "temp": schedule.temprature,
                                             "humidity": schedule.humidity,
                                             "dataSource": schedule.dataSource,
                                             "weekdays": schedule.repeats.split(',')
                                         })
                                 })

        device.messages.forEach(message =>
                                {
                                    send_data.messages.push(
                                        {
                                            "icon": message.icon,
                                            "message": message.message,
                                            "type": message.type,
                                            "isRead": message.isRead,
                                            "datetime": message.datetime,
                                        })
                                })

        device._sensors.forEach(sensor =>
                                {
                                    send_data.sensors.push(
                                        {
                                            "name": sensor.name,
                                            "location": sensor.location === 0 ? "Office" : "Bedroom", // string
                                            "type": sensor.type === 0 ? "OnBoard" : "Wireless", //string
                                            "uid": "213137"
                                        })
                                })

        deviceControllerCPP.pushSettingsToServer(send_data, settingsPush.hasSettings)
    }

    function checkQRurl(url: var) {
        root.device.contactContractor.technicianURL = url;
    }

    function setSystemModeServer(mode_id) {
        if (editModeEnabled(AppSpec.EMSystemMode)) {
            console.log("The system setup is being edited and cannot be updated (mode_id) by the server.")
        } else {
            var modeInt = parseInt(mode_id) - 1;
            //! Vacation will be handled using setVacationServer
            if (modeInt >= AppSpec.Cooling && modeInt <= AppSpec.Off &&
                    modeInt !== AppSpec.Vacation)
                device.systemSetup.systemMode = modeInt;
        }
    }

    function setDesiredTemperatureFromServer(temperature: real) {
        if (editModeEnabled(AppSpec.EMDesiredTemperature)) {
            console.log("The temperature is being edited and cannot be updated by the server.")
            return;
        }

        setDesiredTemperature(temperature);
    }

    //! Set temperature to device (system) and update model.
    function setDesiredTemperature(temperature: real) {
        //! Apply temperature in backend
        deviceControllerCPP.setRequestedTemperature(temperature);

        // Update device temperature when setTemperature is successful.
        device.requestedTemp = temperature;
    }

    function setRequestedHumidityFromServer(humidity: real) {
        if (editModeEnabled(AppSpec.EMRequestedHumidity)) {
            console.log("The humidity is being edited and cannot be updated by the server.")
            return;
        }

        setRequestedHumidity(humidity);
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

    function setSystemSetupServer(settings: var) {

        if (editModeEnabled(AppSpec.EMSystemSetup)) {
            console.log("The system setup is being edited and cannot be updated by the server.")
            return;
        }

        device.systemSetup.heatPumpEmergency = settings.heatPumpEmergency;
        device.systemSetup.heatStage = settings.heatStage;
        device.systemSetup.coolStage = settings.coolStage;
        device.systemSetup.heatPumpOBState = settings.heatPumpOBState;
        device.systemSetup.systemRunDelay = settings.systemRunDelay;
        setSystemAccesseoriesServer(settings.systemAccessories)

        if (settings.type === "traditional")
            setSystemTraditional(settings.coolStage, settings.heatStage);
        else if(settings.type === "heating")
            setSystemHeatOnly(settings.heatStage)
        else if(settings.type === "heat_pump")
            setSystemHeatPump(settings.heatPumpEmergency, settings.heatStage, settings.heatPumpOBState)
        else if(settings.type === "cooling")
            setSystemCoolingOnly(settings.coolStage)
        else
            console.warn("System type unknown", settings.type)
    }

    function setAutoTemperatureFromServer (settings) {

        if (!device)
            return;

        if (editModeEnabled(AppSpec.EMDesiredTemperature)) {
            console.log("The temperature is being edited and cannot be updated by the server.")
            return;
        }

        if (settings.hasOwnProperty("auto_temp_low")) {
            if (device.autoMinReqTemp !== settings.auto_temp_low) {
                device.autoMinReqTemp = settings.auto_temp_low;
                deviceControllerCPP.setAutoMinReqTemp(device.autoMinReqTemp);
            }
        }

        if (settings.hasOwnProperty("auto_temp_high")) {
            if (device.autoMaxReqTemp !== settings.auto_temp_high) {
                device.autoMaxReqTemp = settings.auto_temp_high;
                deviceControllerCPP.setAutoMaxReqTemp(device.autoMaxReqTemp);
            }
        }

    }

    function setAutoMinReqTemp(min) {
        if (device && device.autoMinReqTemp !== min) {
            device.autoMinReqTemp = min;
            deviceControllerCPP.setAutoMinReqTemp(min);
        }
    }

    function setAutoMaxReqTemp(max) {
        if (device && device.autoMaxReqTemp !== max) {
            device.autoMaxReqTemp = max;
            deviceControllerCPP.setAutoMaxReqTemp(max);
        }
    }

    function checkSensors(sensors: var) {
        if (editModeEnabled(AppSpec.EMSensors)) {
            console.log("The sensors are being edited and cannot be updated by the server.")
            return;
        }

        sensors.forEach(sensor => console.log(sensor.location, sensor.name, sensor.type, sensor.uid, sensor.locationsd))
    }

    //! Compare the server schedules and the model schedules and update model based on the server data.
    function setSchedulesFromServer(serverSchedules: var) {
        if (editModeEnabled(AppSpec.EMSchedule)) {
            console.log("The schedules are being edited and cannot be updated by the server.")
            return;
        }

        if (schedulesController)
            schedulesController.setSchedulesFromServer(serverSchedules);
    }

    function setMessagesServer(messages: var) {
        let messagesModel = device.messages;
        console.log("device.messages: ", device.messages.length)

        // Send messages to message controller.
        messageController.setMessagesServer(messages);

    }

    //! Control the push to server with the updateInformation().
    property int _pushUpdateInformationCounter: 0

    //! Reset the _pushUpdateInformationCounter
    property Timer _pushUpdateInformationTimer: Timer {
        repeat: true
        running: true
        interval: 60000

        onTriggered: {
            _pushUpdateInformationCounter = 0;
        }
    }

    //! Read data from system with getMainData method.
    function updateInformation()
    {
        //        console.log("--------------- Start: updateInformation -------------------")
        var result = deviceControllerCPP.getMainData();
        if (!result.temperature)
            return;

        var co2 = result?.iaq ?? 0;
        var co2Id = device?.airQuality(co2) ?? 0;

        // Fahrenheit is more sensitive than Celsius,
        // so for every one degree change,
        // it needs to be sent to the server.
        var isVisualTempChangedF = Math.abs(Math.round(device.currentTemp * 1.8 ) - Math.round((result?.temperature ?? device.currentTemp) * 1.8)) > 0
        var isVisualTempChangedC = Math.abs(Math.round(device.currentTemp * 1.0 ) - Math.round((result?.temperature ?? device.currentTemp) * 1.0)) > 0
        var isVisualHumChanged = Math.abs(Math.round(device.currentHum) - Math.round(result?.humidity ?? device.currentHum)) > 0
        var isCo2IdChanged = device._co2_id !== co2Id;
        var isNeedToPushToServer = isVisualHumChanged ||
                isVisualTempChangedC || isVisualTempChangedF ||
                isCo2IdChanged;

        // should be catched later here
        device.currentHum = result?.humidity ?? 0
        device.currentTemp = result?.temperature ?? 0
        device.co2 = co2 // use iaq as indicator for air quality
        //        device.setting.brightness = result?.brighness ?? 0

        //        device.fan.mode?

        if (isNeedToPushToServer && _pushUpdateInformationCounter < 5) {
            _pushUpdateInformationCounter++;
            pushUpdateToServer(false);
        }

        //        console.log("--------------- End: updateInformation -------------------")
    }

    function updateHoldServer(isHold)
    {

        if (editModeEnabled(AppSpec.EMHold)) {
            console.log("The hold page is being edited and cannot be updated by the server.")
            return;
        }

        updateHold(isHold);
    }

    function updateHold(isHold)
    {
        // TODO should be updated to inform the logics

        if (device.isHold !== isHold)
            device.isHold = isHold;
    }

    function setSystemAccesseoriesServer(settings: var) {
        setSystemAccesseories(settings.mode, AppSpec.accessoriesWireTypeToEnum(settings.wire));
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

    function updateNightModeWithBacklight(isOn : bool) {
        updateNightMode(isOn ? AppSpec.NMOff : AppSpec.NMOn);
    }

    function runNightMode() {
        if (device.nightMode._running) {
            // Apply night mode
            // Set night mode settings
            // LCD should be set to minimum brightness, and ideally disabled.

            var brightness = 5;
            if (ScreenSaverManager.state !== ScreenSaverManager.Timeout) {
                brightness = targetNightModeBrightness;
            }

            setBrightnessInNightMode(brightness, device.setting.volume, false);

        } else {
            console.log("Night mode stopping: revert to model.")
            // revert to model
            if (device)
                setBrightnessInNightMode(device.setting.brightness, device.setting.volume,
                                         device.setting.adaptiveBrightness)

            var backlight = device.backlight;
            if (backlight && device.nightMode.mode === AppSpec.NMOn) {
                var color = device.backlight.backlightFinalColor(backlight.shadeIndex,
                                                                 backlight.hue,
                                                                 backlight.value);
                updateDeviceBacklight(backlight.on, color);
            }
        }

        // Update the cpu governer with the night mode running and screen saver.
        deviceControllerCPP.setCPUGovernor((device.nightMode._running && ScreenSaverManager.state === ScreenSaverManager.Timeout) ?
                                               AppSpec.CPUGpowersave :
                                               AppSpec.CPUGondemand);
        deviceControllerCPP.nightModeControl(device?.nightMode?._running ?? false);
    }

    //! Just use for night mode
    function setBrightnessInNightMode(brightness, volume, adaptive) {
        var send_data = [brightness, volume, device.setting.tempratureUnit, adaptive];
        if (!deviceControllerCPP.setSettings(send_data)){
            console.warn("setting failed");
        }
    }
}
