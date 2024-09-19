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

    property Sync sync: deviceControllerCPP?.sync ?? null

    property int editMode: AppSpec.EMNone

    //! Use stageMode to handle in progress push
    property int stageMode: AppSpec.EMNone

    //! Use LockMode to handle in progress edit
    property int lockMode: AppSpec.EMNone

    property bool initialSetup: false;

    //! Air condition health
    property bool airConditionSensorHealth: false;

    //! Temperature sensor health
    property bool temperatureSensorHealth:  false;

    //! Humidity sensor health
    property bool humiditySensorHealth:  false;

    //! mandatory update
    //! Set to true when in initial setup exist new update
    //! more usage in future like force update with permission
    property bool mandatoryUpdate: false;

    property var  uiSession

    //! Night mode brightness when screen saver is off.
    property real nightModeBrightness: -1
    property real targetNightModeBrightness: Math.min(50, (device.setting.adaptiveBrightness ? deviceControllerCPP.adaptiveBrightness : device.setting.brightness))

    property int testModeType: AppSpec.TestModeType.None

    //! Is the software update checked or not
    property bool checkedSWUpdate: false

    property var internal: QtObject {
        //! This property will hold last returned data from manual first run flow
        property string syncReturnedEmail: ""
        property string syncReturnedZip: ""
    }

    //! TODO: This will be used to retry the service titan fetch operation in case of errors
    //! Used delays in the fetchServiceTitanInformation calls to improve the overall user experience.
    property Timer fetchServiceTitanTimer: Timer {
        interval: 5000 // TODO
        repeat: true
        running: initialSetup && deviceControllerCPP.system.serialNumber.length > 0

        onTriggered: {
            deviceControllerCPP.system.fetchServiceTitanInformation();
        }
    }

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

    //! The screen will gradually (within up to 3 seconds) set the screen brightness to targetNightModebrightness
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


    //! Start a timer to check serial number.
    property Timer checkSNTimer: Timer {
        property int _retrycheckSNTimerInterval: 10000

        repeat: false
        running: false
        interval: _retrycheckSNTimerInterval

        onTriggered: {
            deviceControllerCPP.checkSN();

            if (deviceControllerCPP.system.serialNumber.length === 0) {
                _retrycheckSNTimerInterval += 10000;

                if (_retrycheckSNTimerInterval > 40000)
                    _retrycheckSNTimerInterval = 40000;

            } else {
                _retrycheckSNTimerInterval = 10000;
            }

        }
    }

    property Connections  deviceControllerConnection: Connections {
        target: deviceControllerCPP

        // adding version so we can force the image to refresh the content
        property int version : 0;

        function onCo2SensorStatus(status: bool) {
            if (airConditionSensorHealth !== status) {
                airConditionSensorHealth = status;
            }
        }

        function onTemperatureSensorStatus(status: bool) {
            if (temperatureSensorHealth !== status) {
                temperatureSensorHealth = status;
                deviceControllerCPP.runTemperatureScheme(status);
            }
        }

        function onHumiditySensorStatus(status: bool) {
            if (humiditySensorHealth !== status) {
                humiditySensorHealth = status;
                deviceControllerCPP.runHumidityScheme(status);
            }
        }

        //! Set system mode to auto when
        //! return the system to Auto mode with the default temp range from 68F to 76F.
        function onExitForceOffSystem() {
            if (device.systemSetup._isSystemShutoff) {
                device.systemSetup._isSystemShutoff = false;

                // Move to auto mode with specified values
                setAutoMinReqTemp(20);
                setAutoMaxReqTemp(24.444);
                setSystemModeTo(AppSpec.Auto);
            }
        }

        //! Force off the system (Temperature and humidity)
        function onForceOffSystem() {
            setSystemModeTo(AppSpec.Off);
            device.systemSetup._isSystemShutoff = true;
        }

        function onContractorInfoUpdated(brandName, phoneNumber, iconUrl, url,  techUrl) {

            version++;
            console.log("onContractorInfoUpdated: ", brandName, phoneNumber, iconUrl, url, techUrl, version);

            root.device.contactContractor.brandName     = brandName
            root.device.contactContractor.phoneNumber   = phoneNumber
            root.device.contactContractor.iconSource    = iconUrl === "" ? getFromBrandName(brandName) : (iconUrl + "?version=" + version)
            root.device.contactContractor.qrURL         = url
            //            root.device.contactContractor.technicianURL = techUrl
        }

        //! Logics for check SN:
        //! The checkSN will be continuously executed until a valid serial number is obtained.
        //! The checkSN will be called after initial setup finished and continue to change hasClient to true.
        //! This called when checkSN called in anyway
        function onSnModeChanged(snMode: int) {

            if (deviceControllerCPP.system.serialNumber.length === 0) {
                //! This called when checkSN called in anyway, so the timer should be singleshot.
                checkSNTimer.repeat = false;
                checkSNTimer.start();

            } else if (snMode !== 2) {
                // Has client is true
                checkSNTimer.stop();

                // Check contractor info once has client received and is true
                deviceControllerCPP.checkContractorInfo();

                // Since checkContractorInfo has been invoked, so reset the timer associated
                // with fetching contractor information to delay the checking process (avoid attempt error).
                if (fetchContractorInfoTimer.running)
                    fetchContractorInfoTimer.restart();
            }
        }
    }

    property Connections networkInterface: Connections {
        target: NetworkInterface

        function onHasInternetChanged() {
            deviceControllerCPP.wifiConnected(NetworkInterface.hasInternet);

            // checkSN when the internet is connected.
            if (NetworkInterface.hasInternet) {
                if (startMode !== 0 && startMode !== -1) {
                    if (!checkSNTimer.running) {
                        deviceControllerCPP.checkSN();
                    }
                }

                if (deviceControllerCPP.system.serialNumber.length > 0)
                    fetchContractorInfoTimer.start();

            } else {
                fetchContractorInfoTimer.stop();
            }
        }
    }

    property Connections system: Connections {
        target: deviceControllerCPP.system

        function onSettingsReady(settings) {
            if (settingsPush.isPushing) {
                // what should we do about last time as we updated it but ignored the content!
                console.log("Err: We have some changes that not applied from the server due to pushing in progress.")
                //! we may have 5 seconds gap which can override the changes of Mobile
                //! so we ignore whole incoming value as the time of pushing will be latter
                //! \TODO: when API is ready to send just the actual changes we can remove
                return;
            }

            updateHoldServer(settings.hold)
            updateFanServer(settings.fan)
            setSettingsServer(settings.setting)
            setRequestedHumidityFromServer(settings.humidity)

            // The temperature might change several times due to mode change or temperature, but the last recorded value is the correct one.
            setDesiredTemperatureFromServer(settings.temp)
            setSystemModeServer(settings.mode_id)
            setSchedulesFromServer(settings.schedule)
            setVacationServer(settings.vacation)
            checkSensors(settings.sensors)
            setSystemSetupServer(settings.system)

            // Save settings after fetch
            saveSettings();

        }

        function onAreSettingsFetchedChanged(success) {
            if (success) {
                settingsLoader.interval = 5000;
                console.log("fetching success, back to ", settingsLoader.interval);
            }
            else {
                var intervalNew = settingsLoader.interval * 2;
                if (intervalNew > 60000) {
                    intervalNew = 60000;
                }
                settingsLoader.interval = intervalNew;
                console.log("fetching failed, backing off to ", settingsLoader.interval)
            }

            settingsLoader.isFetching = false;
        }

        function onAppDataReady(data) {
            // This is not a settings section, the QR URL is just part of the information
            updateTechQRurl(data.qr_url);
            setMessagesServer(data.messages)
        }

        //! Update the auto mode settings with the fetch signal.
        function onAutoModeSettingsReady(settings, isValid) {
            if (isValid) {
                setAutoTemperatureFromServer(settings);
                saveSettings();
            }
        }

        //! Update the auto mode settings with the fetch signal.
        function onAutoModePush(isSuccess: bool) {
            console.log("DeviceController.qml: push onAutoModePush, isSuccess: ", isSuccess, stageMode, editMode, lockMode);

            if (isSuccess) {
                stageMode &= ~AppSpec.EMAutoMode;
            }

            settingsPush.isPushing = false;
            console.log("DeviceController.qml: push onAutoModePush", stageMode);

        }

        function onPushSuccess() {
            console.log("DeviceController.qml: onPushSuccess", stageMode, editMode, lockMode)

            if ((root.stageMode & AppSpec.EMAutoMode) === AppSpec.EMAutoMode) {
                stageMode = AppSpec.EMAutoMode;
            } else {
                stageMode = AppSpec.EMNone;
            }

            settingsPush.isPushing = false;

            console.log("DeviceController.qml: Push onPushSuccess", stageMode)

        }

        function onPushFailed() {
            console.log("DeviceController.qml: Push onPushFailed", stageMode, editMode, lockMode)
            settingsPush.isPushing = false;
        }

        function onServiceTitanInformationReady(hasError: bool, isActive : bool,
                                                email : string, zipCode : string) {
            console.log("ServiceTitanInformationReady", hasError, isActive, email, zipCode);

            device.serviceTitan._fetched = !hasError;

            device.serviceTitan.isActive = isActive;

            if (hasError) {
                // Retry to fetch service titan data.

            } else {
                //! TODO: Update service titan model
                // device.serviceTitan.email = email;
                // device.serviceTitan.zipCode = zipCode;

                // Instead of using the stop() function, utilize the running property to break the binding.
                fetchServiceTitanTimer.running = false;
            }
        }

        //! Check update
        function onUpdateNoChecked() {
            checkedSWUpdate = true;
            console.log("udpate checked.")
        }

        //! TODO: replace new model with the current model
        function onWarrantyReplacementFinished(success: bool) {
            // TODO: action for now
            if (success) {
            }
        }

        function onSerialNumberReady() {
            // "If the software update is not currently checked,
            // initiate a check for updates.
            // If the software update is already checked,
            // proceed with the normal update process using a system timer.
            if (!checkedSWUpdate) {
                deviceControllerCPP.system.fetchUpdateInformation(true);
            }
        }

        function onContractorInfoReady(getDataFromServerSuccessfully : bool) {
            fetchContractorInfoTimer._retryFetchContractorInfoTimerInterval = getDataFromServerSuccessfully ? fetchContractorInfoTimer._defaultInterval : 30000;

            fetchContractorInfoTimer.restart();
        }
    }

    property Connections syncConnections: Connections {
        target: sync

        function onUserDataFetched(email:string, name: string) {
            if (!device || !device.userData) return;

            device.userData.email = email;
            device.userData.name = name;

            // save the updated data to file
            saveSettings();
        }

        function onJobInformationReady(success: bool, data: var) {
            if (!success || !device || !device.serviceTitan)
                return;

            device.serviceTitan.fullName = data?.full_name ?? "";
            device.serviceTitan.phone    = data?.phone ?? "";
            device.serviceTitan.email    = data?.email ?? "";

            device.serviceTitan.zipCode  = data?.zip?.code ?? (data?.zip ?? "");

            device.serviceTitan.city   = data?.city?.name ?? (data?.city ?? "");
            device.serviceTitan.state  = data?.state?.short ?? (data?.state ?? "");

            device.serviceTitan.city_id  = data.city?.id ?? -1;
            device.serviceTitan.state_id  = data.state?.id ?? -1;

            device.serviceTitan.address1 = data?.address1 ?? "";
            device.serviceTitan.address2 = data?.address2 ?? "";

        }

        function onZipCodeInfoReady(success: bool, data: var) {
            if (!success || !data  || !device || !device.serviceTitan) {
                internal.syncReturnedZip = "";
                zipCodeInfoReady("Getting zip code information failed.");
                return;
            }

            if (data.code !== device.serviceTitan.zipCode)
                console.warn("onZipCodeInfoReady: zip code returned is different", data.code, device.serviceTitan.zipCode);

            device.serviceTitan.city  = data.city?.name ?? "";
            device.serviceTitan.state  = data.state?.short ?? ""; //data.state?.name ;

            device.serviceTitan.city_id  = data.city?.id ?? -1;
            device.serviceTitan.state_id  = data.state?.id ?? -1;

            internal.syncReturnedZip = data.code;
            zipCodeInfoReady("");
        }

        function onCustomerInfoReady(success: bool, data: var) {
            if (!success || !data || !device || !device.serviceTitan) {
                internal.syncReturnedEmail = "";
                customerInfoReady("Getting customer information failed.");
                return;
            }

            if (data.email !== device.serviceTitan.email)
                console.warn("onCustomerInfoReady: email returned is different", data.email, device.serviceTitan.email);

            console.log("onCustomerInfoReady:", data.membership, data.is_enabled);

            device.serviceTitan.fullName = data.full_name ?? "";
            device.serviceTitan.phone    = data.phone ?? "";

            internal.syncReturnedEmail = data.email;
            customerInfoReady("");
        }

        function onInstalledSuccess() {
            firstRunFlowEnded();
        }

        function onInstallFailed() {
            console.warn("install failed try again.")
        }

    }

    property Timer  settingsPush: Timer {
        repeat: true // should repeat if not pushed
        running: !isPushing &&
                 (root.editMode !== AppSpec.EMNone || root.stageMode !== AppSpec.EMNone) &&
                 deviceControllerCPP.system.areSettingsFetched && !settingsLoader.isFetching
        interval: 500;

        property bool isPushing : false

        onTriggered: {

            isPushing = true;

            console.log("DeviceController.qml: Push, settingsPush timer: ",
                        ", editMode: ", root.editMode,
                        ", stageMode: ", root.stageMode, "lockMode: ", lockMode);

            // Update the stage mode and clear the edit editMode
            // Move all edit mode flags to stage mode, so the push process is in progress
            stageMode = stageMode | editMode;
            editMode = AppSpec.EMNone;

            // deprioritize if only sensorValues there to push auto mode api sooner
            var priorityMode = root.stageMode & ~AppSpec.EMSensorValues;

            // Start push process if stage mode is available
            // push auto mode first if nothing else is staged except EMSensorValues
            // need some delay here between two api pushes so if another one exist we push the other first!
            // then, this will be called after success push
            //! this can be delayed too much if never push success or too much edits
            if (priorityMode === AppSpec.EMAutoMode) {
                pushAutoModeSettingsToServer();
            } else if (root.stageMode !== AppSpec.EMNone){
                try {
                    pushToServer();
                } catch (err) {
                    console.log("DeviceController.qml: Push, error in push to server")
                    isPushing = alse;
                }
            } else {
                isPushing = alse;
            }

            // sensor true fails, in between goes false
            if (root.editMode === AppSpec.EMNone &&
                    root.stageMode === AppSpec.EMNone) {
                console.warn("Something odd hapenned!, restoring the flow.", isPushing)
                isPushing = false;
            }
        }
    }

    property Timer  settingsLoader: Timer {
        repeat: true;
        running: !initialSetup && !isFetching;
        interval: 5000;

        property bool isFetching: false

        onTriggered: {
            isFetching = deviceControllerCPP.system.fetchSettings();
        }
    }

    property Timer  fetchContractorInfoTimer: Timer {
        readonly property int _defaultInterval: 1.1 * 60 * 60 * 1000
        property int _retryFetchContractorInfoTimerInterval: _defaultInterval

        repeat: true;
        running: false
        interval: _retryFetchContractorInfoTimerInterval

        onTriggered: {
            deviceControllerCPP.checkContractorInfo();
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

    //! Once the initial setup process is finished, the app should become
    //! active and automatically navigate to the home page.
    //! This transition should be handled within the home page component.
    signal initialSetupFinished();

    //! first run flow manual data needs in same page
    signal zipCodeInfoReady(var error);
    signal customerInfoReady(var error);

    onStartDeviceRequested: {
        console.log("************** Initialize and create connections **************")
        //! initialize the device and config
        // as well as device io which may TODO refactor later and call it on demand
        deviceControllerCPP.startDevice();

        if (temperatureSensorHealth) {
            deviceControllerCPP.runTemperatureScheme(true);
        }

        if (humiditySensorHealth) {
            deviceControllerCPP.runHumidityScheme(true);
        }

        //! Update TOF sensor status.
        lock(device._lock.isLock, device._lock.pin, true);

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

        // To update the minimum and maximum when model completed
        device.systemSetup.systemModeChanged();

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
        initialSetup = init;
        //! we set to false elsewhere! i.e., in system
        if (init)
            deviceControllerCPP.system.setIsInitialSetup(true);
    }

    function updateEditMode(mode : int) {
        root.editMode |= mode; // add flag
    }

    function updateLockMode(mode : int, enable: bool) {
        if (enable)
            root.lockMode |= mode; // add flag
        else
            root.lockMode &= ~mode; // remove flag
    }

    function editModeEnabled(mode : int) {
        return (root.editMode & mode) === mode ||
                (root.lockMode & mode) === mode ||
                (root.stageMode & mode) === mode;
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

        updateFan(settings.mode, Utils.clampValue(settings.workingPerHour, AppSpec.minimumFanWorking, AppSpec.maximumFanWorking))
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

        // Clamp vacation data.
        var minimumTemperature = Utils.clampValue(settings.min_temp, AppSpec.vacationMinimumTemperatureC,
                                                                     AppSpec.vacationMaximumTemperatureC - AppSpec.minStepTempC);

        // minimumTemperature can not be less than vacationMinimumTemperatureC, so:
        var maximumTemperature = Utils.clampValue(settings.max_temp, minimumTemperature + AppSpec.minStepTempC, AppSpec.vacationMaximumTemperatureC);

        var minimumHumidity = Utils.clampValue(settings.min_humidity, AppSpec.minimumHumidity, AppSpec.maximumHumidity - AppSpec.minStepHum);
        var maximumHumidity = Utils.clampValue(settings.max_humidity, minimumHumidity + AppSpec.minStepHum, AppSpec.maximumHumidity);

        setVacation(minimumTemperature, maximumTemperature, minimumHumidity, maximumHumidity)
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
        if (device.systemSetup._isSystemShutoff) {
            console.log("Ignore system mode, system is shutoff by alert manager.")
            return;
        }

        if (systemMode === AppSpecCPP.Vacation) {
            setVacationOn(true);

        } else if (systemMode >= 0 && systemMode <= AppSpecCPP.Off) {
            //! TODo required actions if any

            checkToUpdateSystemMode(systemMode);
            deviceController.updateEditMode(AppSpec.EMSystemMode);
            // to let all dependant parameters being updated and save all
            Qt.callLater(saveSettings);
        }
    }

    function checkToUpdateSystemMode(systemMode: int) {
        // Deactivate the incompatible schedules when mode changed from server or ui
        uiSession.schedulesController.deactivateIncompatibleSchedules(systemMode);

        device.systemSetup.systemMode = systemMode;
    }

    //! On/off the vacation from server.
    function setVacationOnFromServer(on: bool) {
        device.systemSetup.isVacation = on;
    }

    //! On/off the vacation.
    function setVacationOn(on: bool) {
        device.systemSetup.isVacation = on;
        deviceController.updateEditMode(AppSpec.EMVacation);
        saveSettings();
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
    function setSettings(brightness, volume, temperatureUnit, adaptive, enabledAlerts, enabledNotifications)
    {
        if (!device){
            console.log("corrupted device")
            return false;
        }

        // TODO, prevent setting adaptive for now
        adaptive = false;

        // Mute alerts update locally.
        if (device.setting.enabledAlerts !== enabledAlerts) {
            device.setting.enabledAlerts = enabledAlerts;
        }

        // Mute notifications update locally.
        if (device.setting.enabledNotifications !== enabledNotifications) {
            device.setting.enabledNotifications = enabledNotifications;
        }

        var send_data = [brightness, volume, temperatureUnit, adaptive];
        var current_data = [device.setting.brightness, device.setting.volume,
                            root.temperatureUnit, device.setting.adaptiveBrightness]
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

    //! Use timer to prevent excessive attempts to save file in the short intervals.
    property Timer saveTimer: Timer {
        interval: 100
        running: false
        repeat: false
        onTriggered: {
            if (uiSession.currentFile.length > 0) // we should not save before the app completely loaded
                AppCore.defaultRepo.saveToFile(uiSession.configFilePath);
        }
    }

    //! Save settings to file (configFilePath)
    function saveSettings() {
        if (uiSession) {
            console.log("saveSettings called with edit mode", stageMode, lockMode, uiSession.currentFile)
            saveTimer.restart();

        } else {
            console.log("saveSettings called without uiSession")
        }
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
            // The server interprets temperature data based on the displayed unit (Celsius or Fahrenheit).
            // To maintain accurate control and prevent misinterpretations,
            // the unit should be permanently set to Celsius. so we always use data from device to ignore
            if (!setSettings(settings.brightness, settings.speaker,
                        root.temperatureUnit, settings.brightness_mode,
                             device.setting.enabledAlerts, device.setting.enabledNotifications))
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

    function pushAutoModeSettingsToServer() {
        deviceControllerCPP.pushAutoSettingsToServer(device.autoMinReqTemp, device.autoMaxReqTemp)
    }

    function pushToServer() {
        console.log("DeviceController.qml: Push to server with : pushToServer")
        var send_data = {
            "temp": device.requestedTemp,
            "humidity": device.requestedHum,
            "current_humidity": device.currentHum.toString(),
            "current_temp": device.currentTemp.toString(),
            "co2_id": device._co2_id + 1,
            "hold" : device.isHold,
            "mode_id" : device.systemSetup.systemMode + 1,
            // "auto_temp_high" : device.autoMaxReqTemp,
            // "auto_temp_low" : device.autoMinReqTemp,
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
                "temperatureUnit": 0, // Always celsius (see setSettings in setSettingsServer function)
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

            // Report the app version
            "firmware": {
                "firmware-version": Application.version
            }
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
                                             //!  TODO: remove
                                             "temp": 18.0,
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
                                            "created": message.datetime,
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

    function updateTechQRurl(url: var) {
        var urlIsSame = root.device.contactContractor.technicianURL === url;
        if (!urlIsSame){
            root.device.contactContractor.technicianURL = url;
            saveSettings();
        }
    }

    function setSystemModeServer(mode_id) {
        if (editModeEnabled(AppSpec.EMSystemMode)) {
            console.log("The system setup is being edited and cannot be updated (mode_id) by the server.")
        } else {
            var modeInt = parseInt(mode_id) - 1;
            //! Vacation will be handled using setVacationServer
            if (modeInt >= AppSpec.Cooling && modeInt <= AppSpec.Off &&
                    modeInt !== AppSpec.Vacation) {
                checkToUpdateSystemMode(modeInt);
            }
        }
    }

    function setDesiredTemperatureFromServer(temperature: real) {
        if (editModeEnabled(AppSpec.EMDesiredTemperature)) {
            console.log("The temperature is being edited and cannot be updated by the server.")
            return;
        }

        var temperatureValue = Utils.clampValue(temperature, _minimumTemperatureC, _maximumTemperatureC);
        setDesiredTemperature(temperatureValue);
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

        if (device.systemSetup.systemMode !== AppSpecCPP.Off && device.systemSetup.systemMode !== AppSpecCPP.Vacation)  {
            setSystemModeTo(AppSpecCPP.Cooling)
        }
    }

    function setSystemHeatOnly(stage: int) {
        device.systemSetup.heatStage  = stage;
        device.systemSetup.systemType = AppSpecCPP.HeatingOnly;
        if (device.systemSetup.systemMode !== AppSpecCPP.Off && device.systemSetup.systemMode !== AppSpecCPP.Vacation)  {
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
        setSystemAccessoriesServer(settings.systemAccessories)

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

    //! If the clamping logic has changed, review the corresponding functionality in the
    //! DesiredTemperatureItem class (specifically the updateFirstSecondValues function).
    function setAutoTemperatureFromServer (settings) {

        if (!device)
            return;

        if (editModeEnabled(AppSpec.EMDesiredTemperature)) {
            console.log("The temperature is being edited and auto mode cannot be updated by the server.")
            return;
        }

        var auto_temp_low = AppSpec.defaultAutoMinReqTemp;
        var auto_temp_high = AppSpec.defaultAutoMaxReqTemp;

        // If both auto_temp_low and auto_temp_high are zero, use default values.
        // If auto_temp_low or auto_temp_high is undefined, keep default values.
        if (settings?.auto_temp_low !== 0 || settings?.auto_temp_high !== 0) {
            auto_temp_low = Utils.clampValue(settings?.auto_temp_low ?? AppSpec.defaultAutoMinReqTemp,
                                             AppSpec.autoMinimumTemperatureC,
                                             AppSpec.autoMaximumTemperatureC - AppSpec.autoModeDiffrenceC);

            const minimumSecondarySlider = Math.max(AppSpec.minAutoMaxTemp, auto_temp_low + AppSpec.autoModeDiffrenceC);
            auto_temp_high = Utils.clampValue(settings?.auto_temp_high ?? AppSpec.defaultAutoMaxReqTemp,
                                              minimumSecondarySlider,
                                              AppSpec.autoMaximumTemperatureC);
        }

        setAutoMinReqTemp(auto_temp_low);
        setAutoMaxReqTemp(auto_temp_high);
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
        sensors.forEach(sensor => console.log(sensor.location, sensor.name, sensor.type, sensor.uid, sensor.locationsd))

        if (editModeEnabled(AppSpec.EMSensors)) {
            console.log("The sensors are being edited and cannot be updated by the server.")
            return;
        }

        //! \TODO: add the logic when ready
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
        console.log("device.messages: ", device.messages.length)
        console.log("messages: ", messages.length)

        // Send messages to message controller.
        messageController.setMessagesServer(messages);

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
        //        device.setting.brightness = result?.brightness ?? 0

        //        device.fan.mode?

        if (isNeedToPushToServer) {
            deviceController.updateEditMode(AppSpec.EMSensorValues);
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

    function setSystemAccessoriesServer(settings: var) {
        setSystemAccessories(settings.mode, AppSpec.accessoriesWireTypeToEnum(settings.wire));
    }

    function setSystemAccessories(accType: int, wireType: int) {
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

        return ""
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
        var send_data = [brightness, volume, root.temperatureUnit, adaptive];
        if (!deviceControllerCPP.setSettings(send_data)){
            console.warn("setting failed");
        }
    }

    function forgetDevice() {
        // Remove the save files from the directory.
        console.log("forgetDevice: remove file in ", uiSession.recoveryConfigFilePath, ": ", QSFileIO.removeFile(uiSession.recoveryConfigFilePath));
        console.log("forgetDevice: remove file in ", uiSession.configFilePath, ": ", QSFileIO.removeFile(uiSession.configFilePath));

        deviceControllerCPP.forgetDevice();
    }

    //! Lock/unlock the application
    //! Call from device and server
    function lock(isLock : bool, pin: string, fromServer = false) : bool {
        var force = false;
        if (!isLock && device._lock.pin.length !== 4) {
            console.log("Model was wrong: ", device._lock.pin, ", unlocked without check pin:", pin);
            pin = device._lock.pin;
            force = true;
        } else if (pin.length !== 4) { // Set the pin in lock editMode
            console.log("Pin: ", pin, " has incorrect format.")
            return false;
        }

        if (isLock) {
            device._lock.pin = pin;
        }

        var isPinCorrect = device._lock.pin === pin;
        if (!isLock && !isPinCorrect && (device._lock._masterPIN.length === 4)) {
            console.log("Use master pin to unlock device: ", device._lock._masterPIN);
            isPinCorrect = device._lock._masterPIN === pin;
        }

        console.log("Pin: ", pin, ", isPinCorrect:", isPinCorrect, ", isLock: ", isLock, ", fromServer", fromServer);

        if (isPinCorrect && lockDevice(isLock, force) && !fromServer) {
            Qt.callLater(pushLockUpdates);
        }

        return isPinCorrect;
    }

    //! Update the lock model
    function lockDevice(isLock : bool, force : bool) : bool {
        if (!force && device._lock.isLock === isLock)
            return false;

        // Check has client in lock mode.
        if (isLock && !deviceControllerCPP.system.hasClient()) {
            console.log("The device cannot be locked because there is no active client.")
            return false;
        }

        device._lock.isLock = isLock;
        ScreenSaverManager.lockDevice(isLock);

        uiSession.showHome();

        return true;
    }

    function pushLockUpdates() {
        saveSettings();

        // TODO: Update the server
    }

    //! TODO: maybe need to restart the app or activate the app and go to home
    function firstRunFlowEnded() {
        checkSNTimer.repeat = true;
        checkSNTimer.start();
        initialSetupFinished();
    }

    //! Push initial setup information
    function pushInitialSetupInformation() {
        var send_data = {
            "client": {
                "full_name": device.serviceTitan.fullName,
                "phone": device.serviceTitan.phone,
                "email": device.serviceTitan.email
              },
              "devices": [
                {
                  "sn": deviceControllerCPP.system.serialNumber,
                  "name": device.thermostatName,
                  "address1": device.serviceTitan.address1,
                  "address2": device.serviceTitan.address2,
                  "state": device.serviceTitan.state_id,
                  "city": device.serviceTitan.city_id ,
                  "zip_code": device.serviceTitan.zipCode,
                  "installation_type": device.installationType === AppSpec.ITNewInstallation? "new" : "existing",
                  "resident_type_id": device.residenceType, // or maybe using condition
                  "where_installed_id": device.whereInstalled
                }
              ]
        };

        sync.installDevice(send_data);
    }

    //! as both data needs to be fetched together and each may return error
    //! we do not resend it if last time it was success so we have better chance in total
    function getJobInformationManual() {
        if (internal.syncReturnedZip !== device.serviceTitan.zipCode)
            sync.getAddressInformationManual(device.serviceTitan.zipCode);
        else
            zipCodeInfoReady("");

        if (internal.syncReturnedEmail !== device.serviceTitan.email)
            sync.getCustomerInformationManual(device.serviceTitan.email)
        else
            customerInfoReady("");
    }
}
