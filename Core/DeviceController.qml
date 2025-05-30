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

    property var  uiSession

    property SchedulesController schedulesController

    property MessageController   messageController

    property System system: deviceControllerCPP?.system ?? null

    property Sync sync: deviceControllerCPP?.sync ?? null

    property int editMode: AppSpec.EMNone

    //! Use stageMode to handle in progress push
    property int stageMode: AppSpec.EMNone

    //! Use LockMode to handle in progress edit
    property int lockMode: AppSpec.EMNone

    //! initialSetup: When initialSetup is true the settingsLoader is disabled
    property bool initialSetup: false;

    //! Initialize the `initialSetupNoWIFI` flag with the `initialSetupWithNoWIFI()` function.
    //! The binding to this flag will be broken in `onInstalledSuccess` or other relevant pages.
    property bool initialSetupNoWIFI: system.initialSetupWithNoWIFI();
    property bool isSendingInitialSetupData: false;

    //! Open Device in the alternativeNoWiFiFlow
    property bool alternativeNoWiFiFlow : system.alternativeNoWiFiFlow();

    //! Initialize the `limitedModeRemainigTime` flag with the `limitedModeRemainigTime()` function
    //! The binding to this flag will be broken in `limitedModeTimer`
    property int  limitedModeRemainigTime : system.limitedModeRemainigTime()

    readonly property int  checkSNTryCount: checkSNTimer.tryCount;

    property real           displayCurrentTemp:    18.0

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

    //! Night mode brightness when screen saver is off.
    property real nightModeBrightness: -1
    property real targetNightModeBrightness: Math.min(50, (device.setting.adaptiveBrightness ? deviceControllerCPP.adaptiveBrightness : device.setting.brightness))

    property int testModeType: AppSpec.TestModeType.None

    //! Is the software update checked or not
    property bool checkedSWUpdate: false

    //! Active system mode in dual fuel heating
    property int dfhSystemType: AppSpec.SysTUnknown

    //! Current active system mode.
    property int activeSystemMode: AppSpec.Off

    //! The current status of the system fan (not the device's fan).
    property bool currentSystemFanState: false

    property bool isRunningAuxiliaryHeating: false

    //! Just for initialize, will be brocken
    //! true: The system can automatically update without requiring user confirmation.
    property bool canUpdateSystemSetup: system.isForgottenDeviceStarted()

    property bool isNeedSendInstallLog: false

    property bool nightModeControlEnabled : deviceControllerCPP.deviceAPI.nightModeControlEnabled()

    property var internal: QtObject {
        //! This property will hold last returned data from manual first run flow
        property string syncReturnedEmail: ""
        property string syncReturnedZip: ""
    }

    property int connectedWiFiStrength: AppSpec.wiFiStrength(NetworkInterface.connectedWifi?.strength ?? 0)

    property bool allowPushWifiStrength: true

    property Timer wifiStrengthPushCoolDown: Timer {
        repeat: false
        running: false
        interval: 15 * 60 * 1000

        property bool droppedEvent : false

        onTriggered: {
            if (droppedEvent) {
                root.updateEditMode(AppSpec.EMWiFi);
                root.allowPushWifiStrength = false;
                droppedEvent = false;
                wifiStrengthPushCoolDown.start();
            } else {
                root.allowPushWifiStrength = true;
            }
        }
    }

    //! Active the Network Connection Watchdog when device has not internet for 1 hour.
    property Timer networkWatchdogTimer: Timer {
        repeat: false
        running: (!NetworkInterface.connectedWifi && !NetworkInterface.isWifiDisconnectedManually) ||
                 (NetworkInterface.connectedWifi && !NetworkInterface.hasInternet)
        interval: 1 * 60 * 60 * 1000

        onRunningChanged: {
            console.log("Network Watchdog Timer running changed: ", running, NetworkInterface.connectedWifi,
                        NetworkInterface.isWifiDisconnectedManually, NetworkInterface.hasInternet)
            // Send network log when hasInternet is true
            if (!running) {
                console.log("Save the network log due to Network Watchdog Timer running changed to false.");
                networkWatchdogLoggerTimer.triggered();
            }
        }

        onTriggered: {
            //  Check the isValidNetworkRequestRestart to check in direct calls (triggered).
            if (system.isValidNetworkRequestRestart()) {
                // Restart the app or device
                uiSession.popUps.showCountDownPopUp(
                            qsTr("  Restart Device  "),
                            qsTr("Restarting Device due to network issue."),
                            true,
                            function () {
                                if (system) {
                                    // Save the network log before restarting
                                    system.saveNetworkLogs();

                                    // to prevent another restart call after restarting once unless the condition changed again
                                    system.saveNetworkRequestRestart();

                                    system.rebootDevice();
                                }
                            });
            }
        }
    }

    property Timer networkWatchdogLoggerTimer: Timer {
        repeat: true
        running: NetworkInterface.connectedWifi && !NetworkInterface.hasInternet
        interval: 15 * 60 * 1000

        onTriggered: {
            // Save the network log
            system.saveNetworkLogs();

            sendNetworkWatchdogLogTimer.canRetry = true;
        }
    }

    property Timer sendNetworkWatchdogLogTimer: Timer {
        repeat: true
        running: canRetry && NetworkInterface.hasInternet
        interval: 20000

        property bool canRetry: true

        onTriggered: {
            if (!system.sendNetworkLogs()) {
                canRetry = false;
            }
            console.log("sending network logs finished: ", canRetry)
        }
    }

    //! Try to send install log if needed.
    property Timer sendInstallLogTimer: Timer {
        repeat: false
        running: false
        interval: 20000

        onTriggered: {
            if (root.isNeedSendInstallLog) {
                console.log("sending install logs")
                system.generateInstallLog();
            }
        }
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

    property Timer sendOnceFullDataPacketToServerTimer: Timer {
        interval: 1000
        repeat: false
        running: temperatureSensorHealth && humiditySensorHealth && airConditionSensorHealth

        onTriggered: {
            if (temperatureSensorHealth && humiditySensorHealth && airConditionSensorHealth) {
                ProtoDataManager.sendFullDataPacketToServer();

                //! To break the binding, Only one data transmission is needed.
                running = false;
            }
        }
    }

    //! Timer to check and run the night mode.
    property Timer nightModeControllerTimer: Timer {
        repeat: true
        running: device.nightMode.mode === AppSpec.NMOn && root.nightModeControlEnabled

        interval: 1000

        onTriggered: {
            var currentTime = new Date();
            device.nightMode._running = currentTime.getHours() >= 21 || currentTime.getHours() < 7;
        }
    }

    //! deviceAPI connections
    property Connections deviceAPIConnections: Connections {
        target: deviceControllerCPP.deviceAPI

        //! Manage the night mode
        function onNightModeControlEnabledChanged() {
            root.nightModeControlEnabled = deviceControllerCPP.deviceAPI.nightModeControlEnabled()

            if (!root.nightModeControlEnabled && device.nightMode._running) {
                device.nightMode._running = false;
                runNightMode();
            }
        }
    }

    //! Manage the night mode
    property Connections nightModeController: Connections {
        target: device.nightMode

        enabled: root.nightModeControlEnabled

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

        enabled: device.nightMode._running && root.nightModeControlEnabled

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

        property int tryCount: 0

        onTriggered: {
            if (NetworkInterface.connectedWifi) {
                tryCount++;
                console.log("trying to checkSN:", tryCount)
            }

            deviceControllerCPP.checkSN();

            console.log("initial setup:", initialSetup, ", serial number:", deviceControllerCPP.system.serialNumber)

            if (deviceControllerCPP.system.serialNumber.length === 0) {

                // sending log automatically if fails to get SN on first RUN
                if (tryCount > 0 && initialSetup && NetworkInterface.connectedWifi) {
                    deviceControllerCPP.system.sendFirstRunLog(false);
                }

                _retrycheckSNTimerInterval += 10000;

                if (_retrycheckSNTimerInterval > 40000)
                    _retrycheckSNTimerInterval = 40000;

            } else {
                _retrycheckSNTimerInterval = 10000;
            }

        }
    }

    property Connections controllerConnections: Connections {
        target: root
        function onTemperatureUnitChanged() {
            deviceControllerCPP?.setCelsius(temperatureUnit === AppSpec.TempratureUnit.Cel)
        }

        function onConnectedWiFiStrengthChanged() {
            if (allowPushWifiStrength){
                updateEditMode(AppSpec.EMWiFi);
                allowPushWifiStrength = false;
                wifiStrengthPushCoolDown.restart();
            } else {
                wifiStrengthPushCoolDown.droppedEvent = true;
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

            //! To load the contractor information as soon as the app starts, save it in the model. This avoids waiting for the ContractorInfoUpdated signal.
            saveSettings();
        }

        //! Logics for check SN:
        //! The checkSN will be continuously executed until a valid serial number is obtained.
        //! The checkSN will be called after initial setup finished and continue to change hasClient to true.
        //! This called when checkSN called in anyway
        function onSnModeChanged(snMode: int) {

            if (deviceControllerCPP.system.serialNumber.length === 0) {
                // we should only check for sn when started normally
                if (startMode === 1) {
                    //! This called when checkSN called in anyway, so the timer should be singleshot.
                    checkSNTimer.repeat = false;
                    checkSNTimer.start();
                }

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

        //! Update active system mode in the dual fuel heating
        function onDfhSystemTypeChanged(activeSystemType: int) {
            dfhSystemType = activeSystemType;
        }

        function onMonitoringTemperatureUpdated(monitoringTempratureC: real) {
            ProtoDataManager.setSetTemperature(monitoringTempratureC);
        }

        function onFanWorkChanged(fanState: bool) {
            ProtoDataManager.setCurrentFanStatus(fanState);

            currentSystemFanState = fanState;
            checkLimitedModeRemainigTimer();

        }

        function onCurrentSystemModeChanged(state: int, currentHeatingStage: int, currentCoolingStage: int) {
            activeSystemMode = state;

            ProtoDataManager.setCurrentHeatingStage(currentHeatingStage);
            ProtoDataManager.setCurrentCoolingStage(currentCoolingStage);
        }

        function onManualEmergencyModeUnblockedAfter(miliSecs: int) {
            uiSession.remainigTimeToUnblockSystemMode = miliSecs;
        }

        function onAuxiliaryStatusChanged(isRunning: bool) {
            isRunningAuxiliaryHeating = isRunning;
        }

        function onEmulateWarrantyFlow() {
            console.log("Emulate warranty flow with backdoor");
            root.postWarrantyReplacementFinsihed();
        }
    }


    property Connections networkInterface: Connections {
        target: NetworkInterface

        function onHasInternetChanged() {
            ProtoDataManager.setOnlineStatus(NetworkInterface.hasInternet);

            deviceControllerCPP.wifiConnected(NetworkInterface.hasInternet);

            if (NetworkInterface.hasInternet) {
                if (deviceControllerCPP.system.serialNumber.length > 0) {
                    fetchContractorInfoTimer.start();
                }

            } else {
                // Save network log when hasInternet is false
                if (!NetworkInterface.isWifiDisconnectedManually && !NetworkInterface.connectedWifi) {
                    console.log("Save the network log due to false hasInternet. The auto connection timer started from NetworkInterfase.")
                    networkWatchdogLoggerTimer.triggered();
                }

                fetchContractorInfoTimer.stop();
            }
        }

        function onForgettingAllWifisChanged() {
            console.log("Forgetting Wifis ", (NetworkInterface.forgettingAllWifis ? "started." : "finished."))
        }

        function onConnectedWifiChanged() {
            updateEditMode(AppSpec.EMWiFi);
            allowPushWifiStrength = false;
            wifiStrengthPushCoolDown.droppedEvent = false;
            wifiStrengthPushCoolDown.restart();
        }
    }

    property Connections systemConnection: Connections {
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
            setSchedulesFromServer(settings.schedule)
            setSystemSetupServer(settings.system)
            checkSensors(settings.sensors)

            setVacationServer(settings.vacation)
            setSystemModeServer(settings.mode_id, settings.system.dualFuelManualHeating ?? device.systemSetup.dualFuelManualHeating)

            if (!lockStatePusher.isPushing) {
                updateAppLockState(settings.locked, settings.pin, true);
            }

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

            // We don't need to check the edit mode, because we push all the settings in the post request.
            canUpdateSystemSetup = false;
            system.setIsForgottenDevice(false);

            if ((root.stageMode & AppSpec.EMAutoMode) === AppSpec.EMAutoMode) {
                stageMode = AppSpec.EMAutoMode;
            } else {
                stageMode = AppSpec.EMNone;
            }

            settingsPush.isPushing = false;

            //! Send install log when updte response data is ready.
            if(!sendInstallLogTimer.running && root.isNeedSendInstallLog) {
                sendInstallLogTimer.triggered();
            }

            console.log("DeviceController.qml: Push onPushSuccess", stageMode)

        }

        function onPushFailed() {
            console.log("DeviceController.qml: Push onPushFailed", stageMode, editMode, lockMode)
            settingsPush.isPushing = false;
        }

        function onForgetDeviceRequested() {
            // Forget the device
            uiSession.popUps.showCountDownPopUp(
                        qsTr("  Forget Device  "),
                        qsTr("Forgetting device due to request."),
                        false,
                        function () {
                            if (system) {
                                console.log("Forgetting device due to request.")
                                root.forgetDevice();

                                // reboot after forget is done
                                console.log("rebootDevice due to Forgetting request.")
                                system.rebootDevice();
                            }
                        });
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

        function onSerialNumberReady() {
            // "If the software update is not currently checked,
            // initiate a check for updates.
            // If the software update is already checked,
            // proceed with the normal update process using a system timer.
            if (!checkedSWUpdate) {
                deviceControllerCPP.system.fetchUpdateInformation(true);
            }
        }

        function onInstallLogSent(isSuccess: bool) {
            console.log("Install log sent successfully: ", isSuccess)
            root.isNeedSendInstallLog = !isSuccess;

            if(root.isNeedSendInstallLog) {
                sendInstallLogTimer.start();
            }
        }
    }

    property Connections syncConnections: Connections {
        target: sync

        function onUserDataFetched(email: string, name: string) {
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
            //! to prevent fetching customer info again if not changed later
            internal.syncReturnedEmail   = device.serviceTitan.email;

            device.serviceTitan.zipCode  = data?.zip?.code ?? (data?.zip ?? "");

            device.serviceTitan.country  = data?.country?.name ?? ("US");
            if (device.serviceTitan.country === "United States")
                device.serviceTitan.country = "US";

            //! if there is no new data to fetch than jobid in review page
            //internal.syncReturnedZip = device.serviceTitan.zipCode;

            device.serviceTitan.city   = data?.city?.name ?? (data?.city ?? "");
            device.serviceTitan.state  = data?.state?.short ?? (data?.state ?? "");

            device.serviceTitan.city_id  = data.city?.id ?? -1;
            device.serviceTitan.state_id  = data.state?.id ?? -1;

            device.serviceTitan.address1 = data?.address1 ?? "";
            device.serviceTitan.address2 = data?.address2 ?? "";
        }

        function onZipCodeInfoReady(success: bool, data: var, isNeedRetry: bool) {
            if (!device || !device.serviceTitan) {
                internal.syncReturnedZip = "";
                zipCodeInfoReady("Getting zip code information failed. Device is not ready!", false);
                return;
            }

            if (!success || !data) {
                internal.syncReturnedZip = "";
                zipCodeInfoReady("Getting zip code information failed.", isNeedRetry);
                return;
            }

            if (data.code !== device.serviceTitan.zipCode)
                console.warn("onZipCodeInfoReady: zip code returned is different", data.code, device.serviceTitan.zipCode);

            device.serviceTitan.city  = data.city?.name ?? "";
            device.serviceTitan.state  = data.state?.short ?? ""; //data.state?.name ;

            device.serviceTitan.city_id  = data.city?.id ?? -1;
            device.serviceTitan.state_id  = data.state?.id ?? -1;

            internal.syncReturnedZip = data.code;
            zipCodeInfoReady("", false);
        }

        function onCustomerInfoReady(success: bool, data: var, error: string, isNeedRetry: bool) {
            //! we keep this empty in case of any error so it can be retry
            internal.syncReturnedEmail = "";

            if (!device || !device.serviceTitan) {
                customerInfoReady("Getting customer information failed. Device is not ready!", false);
                return;
            }

            if (!success) {
                customerInfoReady("Getting customer information failed. " + error, isNeedRetry);
                return;
            }

            //! data can be empty without having error on new emails
            if (!data) {
                console.log("Returned data is empty! maybe email is new!");
                customerInfoReady("", false);
                return;
            }

            if (data.email !== device.serviceTitan.email)
                console.warn("onCustomerInfoReady: email returned is different", data.email, device.serviceTitan.email);

            console.log("onCustomerInfoReady:", data.membership, data.is_enabled);

            device.serviceTitan.fullName = data.full_name ?? "";
            device.serviceTitan.phone    = data.phone ?? "";

            internal.syncReturnedEmail = data.email ?? "";
            customerInfoReady("", false);
        }

        function onInstalledSuccess() {


            // Push all settings to the server after the No Wi-Fi installation flow completed.
            // In a normal initial setup, the system setup will be sent from the system setup page.
            if (initialSetupNoWIFI) {
                updateEditMode(AppSpec.EMAll);
            }

            // Send install log
            root.isNeedSendInstallLog = true
            sendInstallLogTimer.start();

            isSendingInitialSetupData = false;
            setInitialSetupNoWIFI(false);
            initialSetupDataPushTimer.retryCounter = 0;

            // Go to home
            firstRunFlowEnded();

            ProtoDataManager.sendFullDataPacketToServer();
        }

        function onInstallFailed(err : string, needToRetry : bool) {
            isSendingInitialSetupData = needToRetry;

            if (!needToRetry || (initialSetupDataPushTimer.retryCounter % 2 === 0)) {
                showInitialSetupPushError(err);
            }

            console.warn("install failed try again.")
        }

        function onLockStatePushed(success: bool, locked: bool) {
            if (success) {
                console.log('Lock state pushed successfully');
                lockStatePusher.stopPushing();
            }
            else {
                lockStatePusher.interval = Math.min(lockStatePusher.interval * 2, 60 * 1000);
                console.error('Pushing app lock state failed, retry internal is ', lockStatePusher.interval);
            }
        }

        function onWarrantyReplacementFinished(success: bool, error: string, needToRetry: bool) {
            if (success) {
                root.postWarrantyReplacementFinsihed();
            }
        }

        function onContractorInfoReady(getDataFromServerSuccessfully : bool) {
            fetchContractorInfoTimer._retryFetchContractorInfoTimerInterval = getDataFromServerSuccessfully ? fetchContractorInfoTimer._defaultInterval : 30000;

            fetchContractorInfoTimer.restart();
        }

        function onSensorValuesPushed(success: bool, pushedTemperatureC: real, pushedHumidity: int, pushedCo2id: int) {
            if (success && (Math.abs(pushedTemperatureC - root.displayCurrentTemp) < 0.001) &&
                    (pushedHumidity === device.currentHum) &&
                    (pushedCo2id === (device._co2_id + 1))) {
                stageMode &= ~AppSpec.EMSensorValues;
            }

            settingsPush.isPushing = false;
        }
    }

    property Connections perfTestServiceConnections: Connections {
        target: PerfTestService

        function onIsTestRunningChanged() {
            updateEditMode(AppSpec.EMDesiredTemperature);
        }
    }

    property Timer initialSetupDataPushTimer: Timer {
        property int retryCounter: 0

        interval: 5000
        repeat: true
        running: isSendingInitialSetupData

        onTriggered: {
            retryCounter++;
            _pushInitialSetupInformation();
        }
    }

    //! To Check the limited mode
    property Timer limitedModeTimer: Timer {
        interval: 30000
        repeat: true
        running: false

        onTriggered: {
            if (limitedModeRemainigTime > 0) {
                limitedModeRemainigTime -= 30000
                system.setLimitedModeRemainigTime(limitedModeRemainigTime);
            }

            checkLimitedModeRemainigTimer();
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
            var priorityMode = root.stageMode &
                    ~AppSpec.EMSensorValues;

            // Start push process if stage mode is available
            // push auto mode first if nothing else is staged except sensor related events like EMSensorValues
            // need some delay here between two api pushes so if another one exist we push the other first!
            // then, this will be called after success push
            //! this can be delayed too much if never push success or too much edits
            if (priorityMode === AppSpec.EMAutoMode) {
                pushAutoModeSettingsToServer();

            } else if (root.stageMode !== AppSpec.EMNone) {
                if (priorityMode !== AppSpec.EMNone) {
                    try {
                        pushToServer();

                    } catch (err) {
                        console.log("DeviceController.qml: Push, error in push to server")
                        isPushing = false;
                    }

                } else if ((root.stageMode & AppSpec.EMSensorValues) === AppSpec.EMSensorValues) {
                    sync.pushSensorValues(root.displayCurrentTemp, device.currentHum, (device._co2_id + 1));

                } else {
                    isPushing = false;
                }
            } else {
                isPushing = false;
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

    property Timer lockStatePusher: Timer {
        property bool isPushing : false
        running: isPushing && !deviceControllerCPP.sync.pushingLockState
        interval: 1000
        onTriggered: sendData()

        function stopPushing() {
            isPushing = false;
            interval = 1000;
        }

        function startPushing() {
            isPushing = true;
            interval = 1000;
            sendData();
        }

        function sendData() {
            deviceControllerCPP.sync.pushLockState(device.lock.pin, device.lock.isLock);
        }
    }

    /* Object Properties
     * ****************************************************************************************/
    deviceControllerCPP: DeviceControllerCPP {
        systemSetup: device.systemSetup

        Component.onCompleted: {
            console.log("device controller constructed", "unit:", temperatureUnit)
            setCelsius(temperatureUnit === AppSpec.TempratureUnit.Cel)
        }
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
    signal zipCodeInfoReady(var error, bool isNeedRetry);
    signal customerInfoReady(var error, bool isNeedRetry);

    //! Show emergency error popup
    signal showEmergencyModeError();

    signal showInitialSetupPushError(err: string);

    onStartDeviceRequested: {
        console.log("************** Initialize and create connections **************");
        //! initialize the device and config
        // as well as device io which may TODO refactor later and call it on demand
        deviceControllerCPP.startDevice();

        if (temperatureSensorHealth) {
            deviceControllerCPP.runTemperatureScheme(true);
        }

        if (humiditySensorHealth) {
            deviceControllerCPP.runHumidityScheme(true);
        }

        // TODO we might call this contitionally
        console.log("************** set the backlight on initialization **************")
        updateDeviceBacklight(device.backlight.on, device.backlight._color);

        var send_data = [device.setting.brightness, device.setting.volume,
                         device.setting.tempratureUnit, device.setting.adaptiveBrightness];
        if (!deviceControllerCPP.setSettings(send_data)){
            console.warn("setting failed");
        }

        if (device.lock?.isLock) {
            console.log("Locking device at start");
            ScreenSaverManager.lockDevice(true);
            uiSession.showHome();
        }

        //! Initialize the ProtoDataManager data with the loaded model.
        ProtoDataManager.setSetHumidity(deviceControllerCPP.effectiveHumidity());
        ProtoDataManager.setMCUTemperature(system.cpuTemperature());
        ProtoDataManager.setSetTemperature(device.requestedTemp);
        ProtoDataManager.setCurrentTemperature(device.currentTemp);
        ProtoDataManager.setCurrentHumidity(device.currentHum);
        ProtoDataManager.setCurrentAirQuality(device._co2_id);
        ProtoDataManager.setLedStatus(device.backlight.on);
        //! TODO
        //! Set default 101325 kPa
        ProtoDataManager.setAirPressure(101325);

        ProtoDataManager.setSystemType(device.systemSetup.systemType);
        setSystemModeToProto();
        ProtoDataManager.setOnlineStatus(false);
    }

    onStopDeviceRequested: {
        deviceControllerCPP.stopDevice();
    }

    Component.onCompleted: {

        checkToUpdateDeviceDateTime();

        // To update the minimum and maximum when model completed
        device.systemSetup.systemModeChanged();

        console.log("* requestedTemp initial: ", device.requestedTemp);
        console.log("* requestedHum initial: ", device.requestedHum);
        console.log("* nightModeControlEnabled: ", nightModeControlEnabled);
        deviceControllerCPP.setRequestedTemperature(device.requestedTemp);
        deviceControllerCPP.setRequestedHumidity(device.requestedHum);
        // TODO what parameters should be initialized here?
        deviceControllerCPP?.setFan(device.fan.mode, device.fan.workingPerHour);
        deviceControllerCPP.setAutoMaxReqTemp(device.autoMaxReqTemp);
        deviceControllerCPP.setAutoMinReqTemp(device.autoMinReqTemp);

        checkLimitedModeRemainigTimer();

    }

    /* Children
     * ****************************************************************************************/

    /* Methods
     * ****************************************************************************************/

    function setInitialSetup(init: bool) {
        // Initial setup should remain true when initial setup finished with `No Wi-Fi` method.
        initialSetup = init || initialSetupNoWIFI;

        //! we set to false elsewhere! i.e., in system
        if (initialSetup)
            deviceControllerCPP.system.setIsInitialSetup(true);
    }

    function postWarrantyReplacementFinsihed() {
        console.log("postWarrantyReplacementFinsihed called to get system setup from server.");
        canUpdateSystemSetup = true;
        sync.resetFetchTime();
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

    function editModeEnabled(mode : int) : bool {
        return ((root.editMode & mode) === mode) ||
                ((root.lockMode & mode) === mode) ||
                ((root.stageMode & mode) === mode);
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

            // Send backlight data to ProtoDataManager
            ProtoDataManager.setLedStatus(isOn);

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

        if (root.temperatureUnit === AppSpec.TempratureUnit.Fah) {
            minimumTemperature = Utils.convertedTemperature(minimumTemperature, AppSpec.TempratureUnit.Fah);
            minimumTemperature = minimumTemperature.toFixed(0);
            minimumTemperature = Utils.fahrenheitToCelsius(minimumTemperature);

            maximumTemperature = Utils.convertedTemperature(maximumTemperature, AppSpec.TempratureUnit.Fah);
            maximumTemperature = maximumTemperature.toFixed(0);
            maximumTemperature = Utils.fahrenheitToCelsius(maximumTemperature);

        } else {
            minimumTemperature = minimumTemperature.toFixed(0);
            maximumTemperature = maximumTemperature.toFixed(0);
        }

        setVacation(minimumTemperature, maximumTemperature, minimumHumidity, maximumHumidity)
        setVacationOnFromServer(settings.is_enable)
    }

    function setVacation(temp_min, temp_max, hum_min, hum_max)
    {
        if (!device)
            return;

        let hasChanges = false;
        if (Math.abs(device.vacation.temp_min - temp_min) > 0.05) {
            device.vacation.temp_min = temp_min;
            hasChanges = true;
        }

        if (Math.abs(device.vacation.temp_max - temp_max) > 0.05) {
            device.vacation.temp_max = temp_max;
            hasChanges = true;
        }

        if (Math.abs(device.vacation.hum_min - hum_min) > 0.05) {
            device.vacation.hum_min  = hum_min;
            hasChanges = true;
        }

        if (Math.abs(device.vacation.hum_max - hum_max) > 0.05) {
            device.vacation.hum_max  = hum_max ;
            hasChanges = true;
        }

        if (hasChanges) {
            deviceControllerCPP.setVacation(temp_min, temp_max, hum_min, hum_max);
            updateEditMode(AppSpec.EMVacation);
            saveSettings();
        }
    }

    function setSystemModeTo(systemMode: int, force = false, dualFuelManualHeating = AppSpecCPP.DFMOff, save = true) : bool
    {
        if (device.systemSetup._isSystemShutoff) {
            console.log("Ignore system mode, system is shutoff by alert manager.")
            return false;
        }

        // Update the system mode
        if (systemMode === AppSpecCPP.Vacation) {
            // In vacation mode, we should keep the model if it is necessary.
            if (device.systemSetup.systemType === AppSpec.DualFuelHeating)
                device.systemSetup.dualFuelManualHeating = dualFuelManualHeating;

            setVacationOn(true);
            setSystemModeToProto();

        } else if (systemMode >= AppSpec.Cooling && systemMode < AppSpec.SMUnknown) {
            //! TODo required actions if any

            if (checkToUpdateSystemMode(systemMode, force)) {
                device.systemSetup.dualFuelManualHeating = dualFuelManualHeating;
                setSystemModeToProto();

                if (save) {
                    updateEditMode(AppSpec.EMSystemMode);
                    // to let all dependant parameters being updated and save all
                    Qt.callLater(saveSettings);
                }

            } else {
                console.log("Core/DeviceController.qml, setSystemModeTo: Ignore system mode due to checkToUpdateSystemMode conditions. ", systemMode);
                return false;
            }

        } else {
            console.log("Core/DeviceController.qml, setSystemModeTo: Wrong system mode! ", systemMode);
            return false;
        }

        return true;
    }

    //! This function should only be called from within the setSystemModeTo function.
    function checkToUpdateSystemMode(systemMode: int, force = false) {
        if (device.systemSetup.systemMode === systemMode) {
            return true;
        }

        // Block due to manual emergency heating.
        if (!force && systemMode !== AppSpec.EmergencyHeat && uiSession.remainigTimeToUnblockSystemMode > 0) {

            showEmergencyModeError();

            console.log("Ignore system mode due to blocking of system mode changes by manual emergency heating.");
            return false;
        }

        device.systemSetup.systemMode = systemMode;

        return true;
    }

    //! On/off the vacation from server.
    function setVacationOnFromServer(on: bool) {
        setVacationOn(on, false)
    }

    //! On/off the vacation.
    function setVacationOn(on: bool, save = true) {
        if (device.systemSetup.isVacation === on) {
            return;
        }

        device.systemSetup.isVacation = on;

        if (save) {
            updateEditMode(AppSpec.EMVacation);
            saveSettings();

        } else {
            // We need to send the desired temperature to the server
            updateEditMode(AppSpec.EMDesiredTemperature);
        }
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
        var autoTempLow = device.autoMinReqTemp;
        var autoTempHigh = device.autoMaxReqTemp;

        if (currentSchedule) {
            if (currentSchedule.systemMode !== AppSpec.Cooling && currentSchedule.systemMode !== AppSpec.Heating) {
                autoTempLow = currentSchedule.minimumTemperature;
                autoTempHigh = currentSchedule.maximumTemperature;
            }
        }

        deviceControllerCPP.pushAutoSettingsToServer(autoTempLow, autoTempHigh)
    }

    function getTemperatureForServer() : real {
        var temperature = device.requestedTemp;

        if (PerfTestService.isTestRunning) {
            temperature = PerfTestService.perfTestTemperatureC();

        } else if (device.systemSetup.isVacation) {
            temperature = (device.vacation.temp_min + device.vacation.temp_max) / 2;

        } else if (currentSchedule) {
            if (currentSchedule.systemMode === AppSpec.Cooling) {
                temperature = currentSchedule.maximumTemperature;

            } else if (currentSchedule.systemMode === AppSpec.Heating) {
                temperature = currentSchedule.minimumTemperature;
            }
        }

        return temperature;
    }

    function pushToServer() {
        console.log("DeviceController.qml: Push to server with : pushToServer")
        var send_data = {
            "temp": getTemperatureForServer(),
            "humidity": device.requestedHum,
            "current_humidity": device.currentHum.toString(),
            "current_temp": root.displayCurrentTemp.toString(),
            "co2_id": device._co2_id + 1,
            "hold" : device.isHold,
            "mode_id" : (device.systemSetup.isVacation ? AppSpec.Vacation : device.systemSetup.systemMode) + 1,
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
                "dualFuelThreshold": device.systemSetup.dualFuelThreshold,
                "isAUXAuto": device.systemSetup.isAUXAuto,
                "dualFuelManualHeating": device.systemSetup.dualFuelManualHeating,
                "dualFuelHeatingModeDefault": device.systemSetup.dualFuelHeatingModeDefault,
                "emergencyMinimumTime": device.systemSetup.emergencyMinimumTime,
                "auxiliaryHeating": device.systemSetup.auxiliaryHeating,
                "useAuxiliaryParallelHeatPump": device.systemSetup.useAuxiliaryParallelHeatPump,
                "driveAux1AndETogether": device.systemSetup.driveAux1AndETogether,
                "driveAuxAsEmergency": device.systemSetup.driveAuxAsEmergency,
                "runFanWithAuxiliary": device.systemSetup.runFanWithAuxiliary,
                "systemAccessories": {
                    "wire": AppSpec.accessoriesWireTypeString(device.systemSetup.systemAccessories.accessoriesWireType),
                    "mode": device.systemSetup.systemAccessories.accessoriesWireType === AppSpec.None ?
                                AppSpec.ATNone : device.systemSetup.systemAccessories.accessoriesType,
                },

                "wifiName": getConnectedWiFiName(),
                "wifiStrength": `${NetworkInterface.connectedWifi?.strength ?? 0}`
            },

            // Report the app version
            "firmware": {
                "firmware-version": Application.version
            }
        }

        if (root.currentSchedule) {
            send_data.running_schedule_id = root.currentSchedule.id;
        }

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

    function setSystemModeServer(mode_id, dualFuelManualHeating) {
        if (editModeEnabled(AppSpec.EMSystemMode)) {
            console.log("The system setup is being edited and cannot be updated (mode_id) by the server.")
        } else {
            var modeInt = parseInt(mode_id) - 1;
            //! Vacation will be handled using setVacationServer
            if (modeInt !== AppSpec.Vacation) {
                setSystemModeTo(modeInt, false, dualFuelManualHeating, false);
            }
        }
    }

    function setDesiredTemperatureFromServer(temperature: real) {
        if (editModeEnabled(AppSpec.EMDesiredTemperature)) {
            console.log("The temperature is being edited and cannot be updated by the server.")
            return;
        }

         // When the performance test is running and has been pushed to the server, the temperature value correspond to the performance test value and should be ignored.
        if (PerfTestService.isTestRunning) {
            console.log("The temperature ignored due to perf test.")
            return;
        }

        // When the vacation is running and has been pushed to the server, the temperature value correspond to the vacation value and should be ignored.
        if (device.systemSetup.isVacation) {
            console.log("The temperature ignored due to vacation mode.")
            return;
        }

        // When the schedule is running and has been pushed to the server, the temperature value correspond to the schedule value and should be ignored.
        if (root.currentSchedule) {
            console.log("The temperature ignored due to running schedule.")
            return;
        }

         // the data will be clamped on ui and if changed it will call setDesiredTemperature for updating truncated version
         // as well as updating clamped version! it will push again if clamped version differs
        if (device.requestedTemp !== temperature) { // compare fuzzy
            device.requestedTemp = temperature;
            root.saveSettings();
        }
    }

    //! Set temperature to device (system) and update model.
    function setDesiredTemperature(temperature: real) {

        let truncatedValue = AppUtilities.getTruncatedvalue(temperature);
        truncatedValue = (temperatureUnit === AppSpec.TempratureUnit.Fah
                          ? Utils.fahrenheitToCelsius(truncatedValue)
                          : truncatedValue);
        //! Apply temperature in backend
        deviceControllerCPP.setRequestedTemperature(truncatedValue);

        var value = (temperatureUnit === AppSpec.TempratureUnit.Fah
                     ? Utils.fahrenheitToCelsius(temperature)
                     : temperature);
        if (Math.abs(device.requestedTemp - value) > 0.05) { // compare fuzzy
            // Update device temperature when setTemperature is successful.
            device.requestedTemp = value;
            root.updateEditMode(AppSpec.EMDesiredTemperature);
            root.saveSettings();
        }
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
        setSystemTypeTo(AppSpecCPP.CoolingOnly);

        if (device.systemSetup.systemMode !== AppSpecCPP.Off && device.systemSetup.systemMode !== AppSpecCPP.Vacation)  {
            setSystemModeTo(AppSpecCPP.Cooling)
        }
    }

    function setSystemHeatOnly(stage: int) {
        device.systemSetup.heatStage  = stage;
        setSystemTypeTo(AppSpecCPP.HeatingOnly);
        if (device.systemSetup.systemMode !== AppSpecCPP.Off && device.systemSetup.systemMode !== AppSpecCPP.Vacation)  {
            setSystemModeTo(AppSpecCPP.Heating)
        }
    }

    function setSystemHeatPump(auxiliaryHeating: bool, stage: int, obState: int,
                               emergencyMinimumTime: int, auxiliaryStages: int,
                               useAuxiliaryParallelHeatPump: bool,
                               driveAux1AndETogether: bool,
                               driveAuxAsEmergency: bool,
                               runFanWithAuxiliary = true) {
        device.systemSetup.auxiliaryHeating = auxiliaryHeating;

        // coolStage controls the Y wires.
        device.systemSetup.coolStage = stage;
        device.systemSetup.heatStage = auxiliaryStages;
        device.systemSetup.heatPumpOBState = obState;

        device.systemSetup.emergencyMinimumTime = emergencyMinimumTime;
        device.systemSetup.useAuxiliaryParallelHeatPump = useAuxiliaryParallelHeatPump;
        device.systemSetup.driveAux1AndETogether = driveAux1AndETogether;
        device.systemSetup.driveAuxAsEmergency = driveAuxAsEmergency;
        device.systemSetup.runFanWithAuxiliary = runFanWithAuxiliary;

        setSystemTypeTo(AppSpecCPP.HeatPump);
    }

    function setSystemTraditional(coolStage: int, heatStage: int) {
        device.systemSetup.coolStage = coolStage;
        device.systemSetup.heatStage = heatStage;
        setSystemTypeTo(AppSpecCPP.Conventional);
    }

    function setSystemDualFuelHeating(heatPumpStage: int, stage: int, obState: int,
                                      dualFuelThreshold: real, isAUXAuto: bool, dualFuelHeatingModeDefault: int) {
        device.systemSetup.isAUXAuto = isAUXAuto;
        device.systemSetup.dualFuelHeatingModeDefault = dualFuelHeatingModeDefault;

        // coolStage controls the Y wires.
        device.systemSetup.coolStage = heatPumpStage;
        device.systemSetup.heatStage = stage;

        device.systemSetup.heatPumpOBState = obState;

        device.systemSetup.dualFuelThreshold = dualFuelThreshold;
        setSystemTypeTo(AppSpec.DualFuelHeating);
    }

    function setSystemTypeTo(systemType: int) {
        var dualFuelManualHeating = AppSpec.DFMOff;

        if (systemType === AppSpecCPP.DualFuelHeating && !device.systemSetup.isAUXAuto && device.systemSetup.systemMode === AppSpec.Heating) {
            switch (device.systemSetup.systemType) {
            case AppSpec.Conventional:
            case AppSpec.HeatingOnly: {
                dualFuelManualHeating = AppSpec.DFMAuxiliary;
            } break;

            case AppSpec.HeatPump: {
                dualFuelManualHeating = AppSpec.DFMHeatPump;

            } break;

            case AppSpec.DualFuelHeating: {
                dualFuelManualHeating = (device.systemSetup.dualFuelManualHeating !== AppSpec.DFMOff) ? device.systemSetup.dualFuelManualHeating : AppSpec.DFMHeatPump;
            } break;

            default: {
                dualFuelManualHeating = AppSpec.DFMOff;
            }
            }
        }

        device.systemSetup.dualFuelManualHeating = dualFuelManualHeating;
        device.systemSetup.systemType = systemType;

        ProtoDataManager.setSystemType(systemType);

        if (device.systemSetup.systemMode === AppSpecCPP.EmergencyHeat &&
                systemType !== AppSpecCPP.HeatPump) {
            switch (systemType) {
            case AppSpec.Conventional:
            case AppSpec.HeatPump:
            case AppSpec.HeatingOnly:
            case AppSpec.DualFuelHeating: {
                setSystemModeTo(AppSpecCPP.Heating, true, device.systemSetup.dualFuelManualHeating);
            } break;

            case AppSpec.CoolingOnly: {
                setSystemModeTo(AppSpecCPP.Cooling, true, device.systemSetup.dualFuelManualHeating);

            } break;
            }
        }

    }


    function setSystemSetupServer(settings: var) {
        if (editModeEnabled(AppSpec.EMSystemSetup)) {
            console.log("The system setup is being edited and cannot be updated by the server.")
            return;
        }

        checkSystemSetupServer(settings);
    }

    //! Checks system setup properties comes from the server for existing changes compared to a device
    function checkSystemSetupServer(settings: var) {
        if (canUpdateSystemSetup) {
            applySystemSetupServer(settings);

            // The applySystemSetupServer function should disregard settings if the edit mode is EMSystemSetup.
            // This is because the server settings are exclusively modified when in EMSystemSetup mode,
            // so system setup update is unnecessary.
            canUpdateSystemSetup = false;

            system.setIsForgottenDevice(false);

            console.log("System setup applied due to forget device:", system.isForgottenDeviceStarted(), " or warranty replacement flow.")

            return;
        }

        var hasChangeParams = [];

        if (AppSpec.systemTypeString(device.systemSetup.systemType) != settings.type)
            hasChangeParams.push("systemType");
        if (device.systemSetup.heatPumpEmergency != settings.heatPumpEmergency)
            hasChangeParams.push("heatPumpEmergency");
        if (device.systemSetup.heatStage != settings.heatStage)
            hasChangeParams.push("heatStage");
        if (device.systemSetup.coolStage != settings.coolStage)
            hasChangeParams.push("coolStage");
        if (device.systemSetup.heatPumpOBState != settings.heatPumpOBState)
            hasChangeParams.push("heatPumpOBState");
        if (device.systemSetup.systemRunDelay != settings.systemRunDelay)
            hasChangeParams.push("systemRunDelay");
        if (device.systemSetup.systemAccessories.accessoriesType != settings.systemAccessories.mode)
            hasChangeParams.push("accessoriesType");
        var accessoriesWireType = AppSpec.accessoriesWireTypeToEnum(settings.systemAccessories.wire);
        if (device.systemSetup.systemAccessories.accessoriesWireType != accessoriesWireType)
            hasChangeParams.push("accessoriesWireType");
        if (Math.abs(device.systemSetup.dualFuelThreshold - settings.dualFuelThreshold) > 0.001)
            hasChangeParams.push("dualFuelThreshold");
        if (device.systemSetup.isAUXAuto != settings.isAUXAuto)
            hasChangeParams.push("isAUXAuto");
        if (device.systemSetup.dualFuelHeatingModeDefault != settings.dualFuelHeatingModeDefault)
            hasChangeParams.push("dualFuelHeatingModeDefault");
        if (settings.hasOwnProperty("emergencyMinimumTime") &&
                device.systemSetup.emergencyMinimumTime != settings.emergencyMinimumTime)
            hasChangeParams.push("emergencyMinimumTime");
        if (settings.hasOwnProperty("useAuxiliaryParallelHeatPump") &&
                device.systemSetup.useAuxiliaryParallelHeatPump != settings.useAuxiliaryParallelHeatPump)
            hasChangeParams.push("useAuxiliaryParallelHeatPump");
        if (settings.hasOwnProperty("driveAux1AndETogether") &&
                device.systemSetup.driveAux1AndETogether != settings.driveAux1AndETogether)
            hasChangeParams.push("driveAux1AndETogether");
        if (settings.hasOwnProperty("driveAuxAsEmergency") &&
                device.systemSetup.driveAuxAsEmergency != settings.driveAuxAsEmergency)
            hasChangeParams.push("driveAuxAsEmergency");
        if (settings.hasOwnProperty("auxiliaryHeating") &&
                device.systemSetup.auxiliaryHeating != settings.auxiliaryHeating)
            hasChangeParams.push("auxiliaryHeating");
        if (settings.hasOwnProperty("runFanWithAuxiliary") &&
                device.systemSetup.runFanWithAuxiliary != settings.runFanWithAuxiliary)
            hasChangeParams.push("runFanWithAuxiliary");

        if (hasChangeParams.length > 0) {
            console.log("Changed paramters are :", hasChangeParams, JSON.stringify(settings))

            var allowUser2Choose = false;
            if (allowUser2Choose) {
                uiSession.popUps.showSystemSetupUpdateConfirmation(settings);
            } else {
                updateEditMode(AppSpec.EMSystemSetup);
                saveSettings();
            }
        }
    }

    function applySystemSetupServer(settings: var) {
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
        else if(settings.type === "heat_pump") {
            setSystemHeatPump(settings.auxiliaryHeating ?? device.systemSetup.auxiliaryHeating,
                              settings.coolStage, settings.heatPumpOBState,
                              settings.emergencyMinimumTime ?? device.systemSetup.emergencyMinimumTime,
                              settings.heatStage,
                              settings.useAuxiliaryParallelHeatPump ?? device.systemSetup.useAuxiliaryParallelHeatPump,
                              settings.driveAux1AndETogether ?? device.systemSetup.driveAux1AndETogether,
                              settings.driveAuxAsEmergency ?? device.systemSetup.driveAuxAsEmergency,
                              settings.runFanWithAuxiliary ?? device.systemSetup.runFanWithAuxiliary)

        } else if(settings.type === "cooling")
            setSystemCoolingOnly(settings.coolStage)
        else if(settings.type === AppSpec.systemTypeString(AppSpec.DualFuelHeating))
            setSystemDualFuelHeating(settings.coolStage, settings.heatStage,
                                     settings.heatPumpOBState,
                                     settings.dualFuelThreshold ?? device.systemSetup.dualFuelThreshold,
                                     settings.isAUXAuto ?? device.systemSetup.isAUXAuto,
                                     settings.dualFuelHeatingModeDefault ?? device.systemSetup.dualFuelHeatingModeDefault);
        else
            console.warn("System type unknown", settings.type);

        //! The settings on the server might be different from the settings on the device.
        //! This could be because the device sent old settings when other changes were happening (e.x. sensor data).
        //! So we should push the new applyed settings.
        //! Pushed from SystemSetupPage (onDestruction)
        // updateEditMode(AppSpec.EMSystemSetup);
        // saveSettings();
    }

    //! If the clamping logic has changed, review the corresponding functionality in the
    //! DesiredTemperatureItem class (specifically the updateFirstSecondValues function).
    function setAutoTemperatureFromServer (settings) {
        if (!device)
            return;

        if (editModeEnabled(AppSpec.EMAutoMode)) {
            console.log("The temperature is being edited and auto mode cannot be updated by the server.")
            return;
        }

        // When the schedule is running and has been pushed to the server, the auto mode values correspond to the schedule values and should be disregarded in auto mode.
        if (root.currentSchedule) {
            console.log("The auto mode temperatures ignored due to running schedule.")
            return;
        }

        var isNeedToPush = false;

        var auto_temp_low = AppSpec.defaultAutoMinReqTemp;

        // Add 0.001 to AppSpec.defaultAutoMaxReqTemp to prevent the temperature from being floored to 73F, the correct value is 74F.
        var auto_temp_high = AppSpec.defaultAutoMaxReqTemp + 0.001;

        // If both auto_temp_low and auto_temp_high are zero, use default values.
        // If auto_temp_low or auto_temp_high is undefined, keep default values.
        if ((settings?.auto_temp_low ?? 0) !== 0 || (settings?.auto_temp_high ?? 0) !== 0) {
            auto_temp_low = settings?.auto_temp_low ?? auto_temp_low;
            auto_temp_high = settings?.auto_temp_high ?? auto_temp_high;

        } else {
            isNeedToPush = true;
        }

         // the data will be clamped on ui and if changed it will call setAutoMinReqTemp and setAutoMaxReqTemp for updating truncated version
         // as well as updating clamped version! it will push again if clamped version differs
        if (Math.abs(auto_temp_low - device.autoMinReqTemp) > 0.1) {
            device.autoMinReqTemp = auto_temp_low;
        }

        if (Math.abs(auto_temp_high - device.autoMaxReqTemp) > 0.1) {
            device.autoMaxReqTemp = auto_temp_high;
        }

        if (isNeedToPush) {
            Qt.callLater(updateEditMode, AppSpec.EMAutoMode);
        }
    }

    //!? minTemperature is UI value.
    function setAutoMinReqTemp(minTemperature: real) {
        let truncatedValue = AppUtilities.getTruncatedvalue(minTemperature);
        truncatedValue = (temperatureUnit === AppSpec.TempratureUnit.Fah
                          ? Utils.fahrenheitToCelsius(truncatedValue)
                          : truncatedValue);
        deviceControllerCPP.setAutoMinReqTemp(truncatedValue);

        var value = (temperatureUnit === AppSpec.TempratureUnit.Fah
                ? Utils.fahrenheitToCelsius(minTemperature)
                : minTemperature);
        if (Math.abs(device.autoMinReqTemp - value) > 0.05 ) {
            device.autoMinReqTemp = value;
            root.updateEditMode(AppSpec.EMAutoMode);
            root.saveSettings();
        }
    }

    //!? maxTemperature is UI value.
    function setAutoMaxReqTemp(maxTemperature: real) {
        let truncatedValue = AppUtilities.getTruncatedvalue(maxTemperature);
        truncatedValue = (temperatureUnit === AppSpec.TempratureUnit.Fah
                          ? Utils.fahrenheitToCelsius(truncatedValue)
                          : truncatedValue);
        deviceControllerCPP.setAutoMaxReqTemp(truncatedValue);

        var value = (temperatureUnit === AppSpec.TempratureUnit.Fah
                ? Utils.fahrenheitToCelsius(maxTemperature)
                : maxTemperature);
        if (Math.abs(device.autoMaxReqTemp - value) > 0.05 ) {
            device.autoMaxReqTemp = value;
            root.updateEditMode(AppSpec.EMAutoMode);
            root.saveSettings();
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

        // Check the lock mode
        if (editModeEnabled(AppSpec.EMSchedule)) {
            console.log("The schedules are being edited and cannot be updated by the server.")
            return;
        }

        if (schedulesController)
            schedulesController.setSchedulesFromServer(serverSchedules);
    }

    function setMessagesServer(messages: var) {
        console.log("Local Messages Retrieved from Server (saved in local): ", device.messages.filter(msg => msg.sourceType === Message.SourceType.Server).length)
        console.log("New messages from server: ", messages.length)

        // Send messages to message controller.
        messageController.setMessagesServer(messages);

    }

    //! Read data from system with getMainData method.
    function updateInformation()
    {
        //        console.log("--------------- Start: updateInformation -------------------")
        var result = deviceControllerCPP.getMainData();
        // if temperature exists roundTemperature will exist for sure
        if (!result.temperature) // see AppSpecCPP.h for temperatureKey & roundTemperatureKey definition
            return;

        var co2 = result?.iaq ?? 0;
        var co2Id = device?.airQuality(co2) ?? 0;

        // Fahrenheit is more sensitive than Celsius,
        // so for every one degree change,
        // it needs to be sent to the server.
        var isVisualTempChangedF = Math.abs(Math.round(root.displayCurrentTemp * 1.8 ) - Math.round((result?.roundTemperature ?? root.displayCurrentTemp) * 1.8)) > 0
        var isVisualTempChangedC = Math.abs(Math.round(root.displayCurrentTemp * 1.0 ) - Math.round((result?.roundTemperature ?? root.displayCurrentTemp) * 1.0)) > 0
        // as we are rounding the temperature in user preference, we allow pushing it when user notices change on device in this way because we only push the round value
        var isRoundedTempChanged = Math.abs(root.displayCurrentTemp - (result?.roundTemperature ?? root.displayCurrentTemp)) > 0.01
        var isVisualHumChanged = Math.abs(Math.round(device.currentHum) - Math.round(result?.humidity ?? device.currentHum)) > 0
        var isCo2IdChanged = device._co2_id !== co2Id;

        // should be catched later here
        device.currentHum = result?.humidity ?? 0
        root.displayCurrentTemp = result?.roundTemperature ?? 0
        device.currentTemp = result?.temperature ?? 0
        device.co2 = co2 // use iaq as indicator for air quality
        //        device.setting.brightness = result?.brightness ?? 0

        //        device.fan.mode?


        ProtoDataManager.setSetHumidity(deviceControllerCPP.effectiveHumidity());
        ProtoDataManager.setMCUTemperature(system.cpuTemperature());
        ProtoDataManager.setCurrentTemperature(root.displayCurrentTemp);
        ProtoDataManager.setCurrentHumidity(device.currentHum);
        ProtoDataManager.setCurrentAirQuality(device._co2_id);

        if (isRoundedTempChanged || isVisualHumChanged || isCo2IdChanged) {
            updateEditMode(AppSpec.EMSensorValues);
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
            "processedTemperature": temperature, // see AppSpecCPP.h for key definition processedTemperatureKey
        }
        deviceControllerCPP.setOverrideMainData(on ? send_data : {})
    }

    function getTestData() {
        return deviceControllerCPP.getMainData();
    }

    function setActivatedSchedule(schedule: ScheduleCPP) {

        if (root.currentSchedule === schedule)
            return;

        var previousSchedule = root.currentSchedule;
        root.currentSchedule = schedule;
        deviceControllerCPP.setActivatedSchedule(schedule);

        // Update the server when currentSchedule is null or changed to a valid schedule with valid id.
        if (!root.currentSchedule || root.currentSchedule.id > -1) {
            var editMode = AppSpec.EMSchedule;

            if (root.currentSchedule || previousSchedule) {
                var updateSinleModeTemperature = false;
                var updateAutoModeTemperature  = false;

                if (root.currentSchedule) {
                    updateSinleModeTemperature = root.currentSchedule.systemMode === AppSpec.Heating || root.currentSchedule.systemMode === AppSpec.Cooling;
                    updateAutoModeTemperature = !updateSinleModeTemperature;
                }

                if (previousSchedule) {
                    if (!updateSinleModeTemperature)
                        updateSinleModeTemperature = previousSchedule.systemMode === AppSpec.Heating || previousSchedule.systemMode === AppSpec.Cooling;

                    if (!updateAutoModeTemperature)
                        updateAutoModeTemperature = previousSchedule.systemMode !== AppSpec.Heating && previousSchedule.systemMode !== AppSpec.Cooling;
                }

                if (updateSinleModeTemperature) {
                    editMode |= AppSpec.EMDesiredTemperature;
                    console.log("Push the DesiredTemperature due to schedule changes.")
                }

                if (updateAutoModeTemperature) {
                    editMode |= AppSpec.EMAutoMode;
                    console.log("Push the auto mode due to schedule changes.")
                }
            }

            updateEditMode(editMode);
        } else
            console.log("current schedule id is invalid.", root.currentSchedule.name);
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

    function removeSaveFiles() {
        let resultRemoveConfigFile = QSFileIO.removeFile(uiSession.configFilePath)
        let resultRemoveRecoveryConfigFile = QSFileIO.removeFile(uiSession.recoveryConfigFilePath)

        console.log("removeSaveFiles: remove file in ", uiSession.configFilePath, ": ", resultRemoveConfigFile);
        console.log("removeSaveFiles: remove file in ", uiSession.recoveryConfigFilePath, ": ", resultRemoveRecoveryConfigFile);
    }

    function forgetDevice() {
        removeSaveFiles();
        deviceControllerCPP.forgetDevice();
    }

    function resetDeviceToFactory() {
        console.log("resetDeviceToFactory: start reset factory process");
        removeSaveFiles();
        deviceControllerCPP.resetToFactorySetting();

        if (system) {
            system.rebootDevice(true);
        }
    }

    //! Lock/unlock the application
    //! Call from device and server
    function updateAppLockState(isLock : bool, pin: string, fromServer = false) : bool {
        if (pin?.length !== 4) {
            console.log("Pin: ", pin, " has incorrect format.")
            return false;
        }

        let isPinCorrect = isLock || device.lock.pin === pin;

        if (!isLock && !isPinCorrect && (device.lock._masterPIN.length === 4)) {
            console.log("Use master pin to unlock device: ", device.lock._masterPIN);
            isPinCorrect = device.lock._masterPIN === pin;
            if (isPinCorrect) {
                pin = device.lock.pin;
            }
        }

        console.log("Pin: ", pin, ", isPinCorrect:", isPinCorrect, ", isLock: ", isLock, ", fromServer", fromServer);

        if (isPinCorrect && lockDevice(isLock, pin) && !fromServer) {
            Qt.callLater(pushLockUpdates);
        }

        // During the initial setup, manual device locking is not allowed.
        // Therefore, unlocking the device can only be initiated through the emergency unlock process.
        if (!isLock && isPinCorrect)
            setAlternativeNoWiFiFlow(true);

        return isPinCorrect;
    }

    //! Update the lock model
    function lockDevice(isLock : bool, pin: string) : bool {
        if (device.lock.isLock === isLock && device.lock.pin == pin) {
            console.log("No change in app lock status, ignoring: ", isLock);
            return false;
        }

        // Check has client in lock mode.
        if (isLock && !deviceControllerCPP.system.hasClient()) {
            console.log("The device cannot be locked because there is no active client.");
            return false;
        }

        console.log("Updating App lock state:", isLock);

        device.lock.pin = pin;
        device.lock.isLock = isLock;
        saveSettings();

        ScreenSaverManager.lockDevice(isLock);
        uiSession.showHome();

        return true;
    }

    function pushLockUpdates() {
        lockStatePusher.startPushing();
    }

    //! TODO: maybe need to restart the app or activate the app and go to home
    function firstRunFlowEnded() {
        checkSNTimer.repeat = true;
        checkSNTimer.start();

        initialSetupFinished();
    }

    function pushInitialSetupInformation() {
        if (NetworkInterface.hasInternet) {
            isSendingInitialSetupData = true;
            initialSetupDataPushTimer.retryCounter = 0;
            initialSetupDataPushTimer.triggered();

        } else {
            showInitialSetupPushError(deviceInternetError());
        }
    }

    //! Push initial setup information
    function _pushInitialSetupInformation() {
        // Initialize the client object
        var clientData = {};

        clientData.email = device.serviceTitan.email;
        // Add fields conditionally
        if (device.serviceTitan.fullName) {
            clientData.full_name = device.serviceTitan.fullName;
        }
        if (device.serviceTitan.phone) {
            clientData.phone = device.serviceTitan.phone;
        }

        // Initialize the devices array
        var devicesData = [];

        // construct object for device with minimal data required
        var deviceObj = _prepareAddressPacket();

        deviceObj.sn = deviceControllerCPP.system.serialNumber;
        deviceObj.installation_type = (device.installationType === AppSpec.ITNewInstallation) ? "new" : "existing";
        deviceObj.resident_type_id = device.residenceType; // or maybe using condition
        deviceObj.where_installed_id = device.whereInstalled;

        //! add dynamic fileds
        if (device.thermostatName) {
            deviceObj.name = device.thermostatName;
        }

        // Add the constructed device object to the devices array
        devicesData.push(deviceObj);

        // Now create the final data structure
        var finalData = {
            client: clientData,
            devices: devicesData
        };

        var send_data = {
              "client": clientData,
              "devices": devicesData
        };

        // Send the jobNumber as job_id when the device is installed using jobNumber.
        // If jobNumber is used for installation, isSTManualMode should be set to false.
        if (!device.serviceTitan.isSTManualMode && device.serviceTitan.jobNumber.length > 0)
            send_data.job_id = device.serviceTitan.jobNumber;

        sync.installDevice(send_data);
    }

    //! we do not resend it if last time it was success so we have better chance in total
    function getZipCodeJobInformationManual(zipCode = device.serviceTitan.zipCode) {
        if (internal.syncReturnedZip !== zipCode)
            sync.getAddressInformationManual(zipCode);
        else
            zipCodeInfoReady("", false);
    }

    //! we do not resend it if last time it was success so we have better chance in total
    function getEmailJobInformationManual() {
        if (internal.syncReturnedEmail !== device.serviceTitan.email)
            sync.getCustomerInformationManual(device.serviceTitan.email);
        else
            customerInfoReady("", false);
    }

    function setAlternativeNoWiFiFlow(to : bool) {
        if (initialSetupNoWIFI) {
            alternativeNoWiFiFlow = to;
            system.setAlternativeNoWiFiFlow(to);

            if (to)
                Qt.callLater(uiSession.showHome);
        }
    }

    function setInitialSetupNoWIFI(isnw : bool) {
        initialSetupNoWIFI = isnw;

        if (initialSetupNoWIFI)
            uiSession.goToInitialSetupNoWIFIMode();

        checkLimitedModeRemainigTimer();
    }

    function checkLimitedModeRemainigTimer() {
        // Limited mode will count when relays is ON and device start to work
        if (initialSetupNoWIFI && currentSystemFanState && limitedModeRemainigTime > 0)
            limitedModeTimer.start();
        else 
            limitedModeTimer.stop();
    }

    function setSystemModeToProto() {
        var sysMode = device.systemSetup.systemMode;

        if (device.systemSetup.isVacation) {
            sysMode = AppSpec.Vacation;
        }

        ProtoDataManager.setRunningMode(sysMode);
    }

    function getConnectedWiFiName() : string {
        if (NetworkInterface.connectedWifi) {
            if (NetworkInterface.connectedWifi.ssid.length > 0) {
                return NetworkInterface.connectedWifi.ssid;
            }

            if (NetworkInterface.connectedWifi.bssid.length > 0) {
                return NetworkInterface.connectedWifi.bssid;
            }

            return "N/A";
        }

        return "None"
    }

    function updateAddressInformation() {
        var deviceObj = _prepareAddressPacket();
        deviceObj.sn = deviceControllerCPP.system.serialNumber;

        sync.updateAddressInformationManual(deviceObj);
    }

    function _prepareAddressPacket() : var {
        var deviceObj = {
         "zip_code": device.serviceTitan.zipCode.toUpperCase(),
         "country": AppSpec.supportedCountries.indexOf(device.serviceTitan.country) + 1
        }

        if (device.serviceTitan.address1) {
            deviceObj.address1 = device.serviceTitan.address1;
        }
        if (device.serviceTitan.address2) {
            deviceObj.address2 = device.serviceTitan.address2;
        }
        if (device.serviceTitan.state_id > -1) {
            deviceObj.state = device.serviceTitan.state_id;
        }
        if (device.serviceTitan.city_id > -1) {
            deviceObj.city = device.serviceTitan.city_id;
        }

        return deviceObj;
    }

    //! Check the device date time and try to update it from server.
    function checkToUpdateDeviceDateTime() {
        let rebootFlag = root.system.getRestartFlag()

        if (DateTimeManager.autoUpdateTime === false && rebootFlag === false) {
            uiSession.popUps.showManualDateTimeWarningPopup();

        } else if (DateTimeManager.autoUpdateTime === false) {
            // To Save the time difference after update/restart.
            DateTimeManager.updateTimeDiffrenceBasedonServer();
        }

        if (rebootFlag) {
            root.system.removeRestartFlag()
        }
    }
}
