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

    property int editMode: AppSpec.EMNone

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

    property Connections sync: Connections {
        target: deviceControllerCPP.system

        function onSettingsReady(settings) {
            if (!deviceControllerCPP.system.canFetchServer || settingsPush.running || settingsPushRetry.running) {
                console.log("We have some changes that not applied on the server.")
                return;
            }

            // should we ignore on some cases?
            console.log("loaded settings sn: ", settings.sn, "%%%%%%%%%%%%%%%%%%%%%%%");
            checkQRurl(settings.qr_url)
            updateHoldServer(settings.hold)
            updateFanServer(settings.fan)
            setVacationServer(settings.vacation)
            setRequestedHumidityFromServer(settings.humidity)
            setDesiredTemperatureFromServer(settings.temp)
            checkMessages(settings.messages)
            setSchedulesFromServer(settings.schedule)
            checkSensors(settings.sensors)
            setSettingsServer(settings.setting)
            setSystemSetupServer(settings.system)
        }

        function onCanFetchServerChanged() {
            if (deviceControllerCPP.system.canFetchServer) {
                settingsPushRetry.failed = false;
                settingsPushRetry.interval = 5000;
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
        running: false;
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
        interval: 5000

        onTriggered: {
           root.editMode = AppSpec.EMNone;
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

        settingsLoader.start();
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


    function updateEditMode(editMode : int) {
        console.log("editMode = ", editMode);
        if (editMode !== AppSpec.EMNone) {
            root.editMode = editMode;

        } else {
            editModeTimer.start();
        }
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

    function updateFanServer(settings : var) {

        if (editMode === AppSpec.EMFan) {
            console.log("The fan page is being edited and cannot be updated by the server.")
            return;
        }

        console.log("updateFanSettings")
        updateFan(settings.mode, settings.workingPerHour)
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

    function setVacationServer(settings : var)
    {
        if (editMode === AppSpec.EMVacation) {
            console.log("The vacation is being edited and cannot be updated by the server.")
            return;
        }

        console.log("setVacationServer")
        setVacation(settings.min_temp, settings.max_temp, settings.min_humidity, settings.max_humidity)
        setVacationOn(settings.is_enable)
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

    function finalizeSettings() {
        if (!settingsPush.running)
            settingsPush.start()
    }

    function setSettingsServer(settings: var) {
        if (editMode !== AppSpec.EMSettings) {
            console.log("setSettingsServer")
            setSettings(settings.brightness, settings.speaker, settings.temperatureUnit,
                        settings.timeFormat, false, settings.brightness_mode)
            device.setting.currentTimezone = settings.currentTimezone;
            device.setting.effectDst = settings.effectDst;
        } else {
            console.log("The system settings is being edited and cannot be updated by the server.")
        }
        setBacklightServer(settings.backlight)
    }

    function setBacklightServer(settings: var) {
        if (editMode === AppSpec.EMBacklight) {
            console.log("The backlight is being edited and cannot be updated by the server.")
            return;
        }

        console.log("setBacklightServer")
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
            "hold" : device._isHold,
            "mode_id" : device.systemSetup.systemMode,
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
                "timeFormat": device.setting.timeFormat === AppSpec.TimeFormat.Hour12 ? 1 : 0,
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
                "type": device.systemSetup.systemType === 0 ? "traditional" : " traditional",
                "coolStage": device.systemSetup.coolStage,
                "heatStage": device.systemSetup.heatStage,
                "heatPumpOBState": device.systemSetup.heatPumpOBState,
                "heatPumpEmergency": device.systemSetup.heatPumpEmergency,
                "systemRunDelay": device.systemSetup.systemRunDelay,
                "systemAccessories": {
                    "wire": device.systemSetup.systemAccessories.accessoriesWireType === 0 ? "Tw1" : "none",
                    "mode": device.systemSetup.systemAccessories.accessoriesType,
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


        deviceControllerCPP.pushSettingsToServer(send_data)
    }

    function checkQRurl(url: var) {
        console.log("checkQRurl", url)
    }

    function setDesiredTemperatureFromServer(temperature: real) {
        if (editMode === AppSpec.EMDesiredTemperature) {
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
        if (editMode === AppSpec.EMRequestedHumidity) {
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

        if (editMode === AppSpec.EMSystemSetup) {
            console.log("The system setup is being edited and cannot be updated by the server.")
            return;
        }

        console.log("setSystemSetupServer")
        device.systemSetup.heatPumpEmergency = settings.heatPumpEmergency;
        device.systemSetup.heatStage = settings.heatStage;
        device.systemSetup.coolStage = settings.coolStage;
        device.systemSetup.heatPumpOBState = settings.heatPumpOBState;
        device.systemSetup.systemRunDelay = settings.systemRunDelay;
        setSystemAccesseoriesServer(settings.systemAccessories)
        if (settings.type === "traditional")
            setSystemTraditional(settings.coolStage, settings.heatStage);
        else
            ;// TODO
    }

    function checkSensors(sensors: var) {
        if (editMode === AppSpec.EMSensors) {
            console.log("The sensors are being edited and cannot be updated by the server.")
            return;
        }

        console.log("checkSensors", sensors.length)
        sensors.forEach(sensor => console.log(sensor.location, sensor.name, sensor.type, sensor.uid, sensor.locationsd))
    }

    //! Compare the server schedules and the model schedules and update model based on the server data.
    function setSchedulesFromServer(serverSchedules: var) {
        if (editMode === AppSpec.EMSchedule) {
            console.log("The schedules are being edited and cannot be updated by the server.")
            return;
        }

        console.log("checkSchedules", serverSchedules.length)

        if (schedulesController)
            schedulesController.setSchedulesFromServer(serverSchedules);
    }

    function checkMessages(messages: var) {
        console.log("checkMessages", messages.length)
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

    function updateHoldServer(isHold)
    {

        if (editMode === AppSpec.EMHold) {
            console.log("The hold page is being edited and cannot be updated by the server.")
            return;
        }

        updateHold(isHold);
    }

    function updateHold(isHold)
    {
        // TODO should be updated to inform the logics

        device._isHold = isHold;
    }

    function setSystemAccesseoriesServer(settings: var) {
        setSystemAccesseories(settings.mode, settings.wire);
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
}
