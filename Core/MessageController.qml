import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * MessageController: Add, remove and manage messages.
 * ***********************************************************************************************/
QtObject {
    id: _root

    /* Property Declarations
     * ****************************************************************************************/

    //! uiSession to get model, deviceController and hasUpdateNotification
    //! Use 'var' type to avoid cyclic dependency
    property var uiSession

    property I_Device device: uiSession.appModel

    property I_DeviceController deviceController: uiSession.deviceController

    property bool activeAlerts: false

    property bool is_control_alert_feature_enable : deviceController.system.controlAlertEnabled;

    //! alertInterval: Reshow specific alerts every 24 hours (if exist),
    //! but only if they haven't been displayed in the past 24 hours.
    readonly property int alertInterval: 24 * 60 * 60 * 1000

    //! Weekly: Use in air condition
    readonly property int weeklyAlertInterval: 7 * 24 * 60 * 60 * 1000

    //! 24 Hours: Used in fan alert
    readonly property int dailyAlertInterval: 24 * 60 * 60 * 1000

    //! 6 Hours: Used in Temperature and humidity sensor
    readonly property int sixHoursAlertInterval: 6 * 60 * 60 * 1000

    //! Keep the last read
    //! map <message, last read time>
    property var lastRead: ([])

    //! Keep the show/open messages
    property var messagesShowing: []

    //! Sending alerts to server
    property var sendingAlertsToServer: []

    //! Active alerts after model is loaded
    property Timer activeAlertsTimer: Timer {
        running: device
        repeat: false

        //! Activation of alerts after 10 seconds of program start.
        interval: 10 * 1000

        onTriggered: {
            activeAlerts = true;

            // Remove the ability to show unread alerts
            if (false) {
                // Show messages that isRead is false
                device.messages.forEach(message => {
                                            if (!(message?.isRead ?? true)) {
                                                newMessageReceived(message);
                                            }
                                        });
            }

            // Find alerts that were not successfully delivered to the server.
            device.messages.forEach(message => {
                                        if (message.id === -1) {
                                            checkMessagesForServer(message)
                                        }
                                    });
        }
    }

    //! Check wifi/internet after model is loaded
    //! And start it when wifi or Internet disconnected
    property Timer checkInternetTimer: Timer {
        running: device
        repeat: false

        // Do not show alert on device during first 3 hours of lost connection.
        interval: 3 * 60 * 60 * 1000

        onTriggered: {
            if (checkWifiConnection())
                if(checkInternetConnection())
                    closeWifiInternetAlert();
        }
    }

    //! Trigger an alert if the auxiliary system runs continuously for more than one hour.
    //! TODO: Add more control for: In Case the Customer Ignores the message by clicking Ok or X the alert message should be repeated once again after 1 hour if the Aux heating was still running non stop during that second hour.
    property Timer auxiliaryRunningTimer: Timer {
        repeat: true
        running: deviceController.isRunningAuxiliaryHeating
        interval: 1 * 60 * 60 * 1000

        onTriggered: {
            var alertMessage = "Auxiliary heating is running non stop for 1 hour, if this is normal for your HVAC system ignore the alert, otherwise please contact your Contractor";
            addNewMessageFromData(Message.Type.SystemAlert, alertMessage, DateTimeManager.nowUTC());
        }
    }

    //! Send alert to server
    property Timer pushAlertToServerTimer: Timer {
        running: false
        repeat: false

        interval: 6 * 1000

        onTriggered: {
            sendAlertToServer();
        }
    }

    Component.onCompleted: {
        // Filter out null/undefined messages from file data.
        device.messages = device.messages.filter(message => message !== null && message !== undefined);

        //! To improve efficiency, remove messages if exists more than AppSpec.messagesLimits messages.
        if (device.messages.length > AppSpec.messagesLimits) {
            var deletedMessages = device.messages.splice(AppSpec.messagesLimits);

            deletedMessages.forEach(msg => {
                                        console.log(`message id: ${msg.id}, message: ${msg.message} - removed to improve efficiency.`)
                                    });
        }

        device.messagesChanged();
    }

    /* Signals
     * ****************************************************************************************/
    signal newMessageReceived(Message message)

    signal showMessage(Message message)

    //! Show wifi/Internet connection alert.
    signal showWifiInternetAlert(message: string, dateTime: string)

    signal closeWifiInternetAlert();

    /* Functions
     * ****************************************************************************************/
    function setMessagesServer(messages: var) {
        // messages uid

        if (!Array.isArray(messages)) {
            console.log("Invalid server input. Expected arrays (messages).");
            return;
        }

        // Delete messages from model when a cloud message is removed.
        {}

        messages.forEach(message => {

                             //! Check the necessary fields in the message.
                             if (!message.hasOwnProperty("message_id") || !message.hasOwnProperty("message") || !message.hasOwnProperty("created") || !message.hasOwnProperty("type")) {
                                 console.log("Message has wrong format. ", JSON.stringify(message), message.hasOwnProperty("message_id"),
                                             message.hasOwnProperty("message"), message.hasOwnProperty("created"), message.hasOwnProperty("type"));
                                 return;
                             }

                             if (message.message === "")
                             return;

                             // we need this for now as server sents every message only once, so it should be false so user don't lose it
                             message.is_read = false;

                             var messageDatetime = message.created === null ? "" : message.created;

                             // Type by default is Notification, we are using 3 for notification but server sends 2.
                             var type = (message.type === Message.Type.SystemNotification) ? Message.Type.Notification : (message?.type ?? Message.Type.Notification);

                             if (type !== Message.Type.Notification) {
                                 console.info(`Message from server: type mismatch, type is ${type}, id is ${message.message_id}`);
                             }

                             // Find Schedule in the model
                             var foundMessage = device.messages.find(messageModel => (messageModel.sourceType === Message.SourceType.Server &&
                                                                                      (message.message_id === messageModel.id ||

                                                                                       // To compatible with the old saved messages
                                                                                       (messageModel.id < 0 &&
                                                                                        messageModel.message === message.message &&
                                                                                        messageModel.datetime === messageDatetime &&
                                                                                        messageModel.type === type))
                                                                                      ));

                             if (foundMessage) {
                                 // is_read in the server is wrong. So I use the isRead condition from the local.
                                 // foundMessage.isRead = message.is_read;
                                 foundMessage.id = message.message_id ?? foundMessage.id;

                                 console.log("Message found: ", foundMessage.id, foundMessage.message, ", ignored.")

                             } else { // new message, TODO: Check empty message
                                 let icon = (message.icon === null) ? "" : message.icon;
                                 addNewMessageFromData(type, message.message, message.created, message?.is_read ?? false,
                                                       icon, message.message_id, Message.SourceType.Server, message?.title ?? "");
                             }
                         });
    }

    function addNewMessageFromData(type, message, datetime, isRead = false, icon = "", id = -1, sourceType = Message.SourceType.Device, title = "")
    {
        if (message.length === 0) {
            console.log("addNewMessageFromData: The message is empty!")
            return;
        }

        //! We should show server messages immediately, If the device receives messages from the server but ignore them,
        //! the device will permanently lose those messages.
        if (!activeAlerts && sourceType !== Message.SourceType.Server) {
            console.log("ignored message: ______________________________________\n", "type : ", type, ",message:", message, "\n----------------------------------------------");
            return;
        }

        // notifyUser: Save message to model and hide from the user's view
        var notifyUser = true;

        var modifiedIsRead = isRead;

        // Check Alerts with enabledAlerts in settings
        if ((!device.setting.enabledAlerts && is_control_alert_feature_enable) &&
                (type === Message.Type.SystemNotification ||
                 type === Message.Type.Alert)) {
            // Temporary, can mark messages received from the server as "read" to prevent their display upon alerts re-enablement.
            if (sourceType === Message.SourceType.Server) {
                notifyUser = false;
                modifiedIsRead = true;

            } else {
                console.log("Ignore alerts due to settings: ______________________________________\n", "type : ", type, ",message:", message, "\n----------------------------------------------");
                return;
            }
        }

        // Check Notifications with enabledNotifications in settings
        if (!device.setting.enabledNotifications && type === Message.Type.Notification) {
            // Temporary, can mark messages received from the server as "read" to prevent their display upon notification re-enablement.
            if (sourceType === Message.SourceType.Server) {
                notifyUser = false;
                modifiedIsRead = true;

            } else {
                console.log("Ignore notifications due to settings: ______________________________________\n", "type : ", type, ",message:", message, "\n----------------------------------------------");
                return;
            }
        }

        var newMessage;
        if (type === Message.Type.SystemNotification) {
            // To avoid saving to file
            newMessage = QSSerializer.createQSObject("Message", ["Stherm", "QtQuickStream"])
            newMessage._qsRepo = null;

        } else {
            newMessage = QSSerializer.createQSObject("Message", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
            newMessage._qsRepo = AppCore.defaultRepo;
        }

        newMessage.type = type;
        newMessage.message = message;

        newMessage.title   = title;
        newMessage.datetime = datetime;
        newMessage.isRead = modifiedIsRead;
        newMessage.sourceType = sourceType;
        newMessage.id = id;

        var hasMessagesChanges = false;
        if (type !== Message.Type.SystemNotification) {
            device.messages.unshift(newMessage);
            hasMessagesChanges = true;
        }

        if (notifyUser)
            newMessageReceived(newMessage);

        //! To improve efficiency, remove messages if exists more than AppSpec.messagesLimits messages.
        if (device.messages.length > AppSpec.messagesLimits) {
            var deletedMessages = device.messages.splice(AppSpec.messagesLimits);
            hasMessagesChanges = true;
            deletedMessages.forEach(msg => {
                                        console.log(`message id: ${msg.id}, message: ${msg.message} - removed to improve efficiency.`)
                                    });
        }

        if (hasMessagesChanges) {
            device.messagesChanged();

            // Messages are not being synchronized with the server for now.
            deviceController.saveSettings();
        }

        checkMessagesForServer(newMessage);
    }

    function addNewMessage(message: Message)
    {
        if (!activeAlerts) {
            console.log("ignored message: ______________________________________\n", "message:", message, "\n----------------------------------------------");
            return;
        }

        device.messages.push(message);
        device.messagesChanged();
    }

    function removeMessage(message: Message)
    {
        var msgIndex = device.messages.findIndex((element, index) => element === message);
        if (msgIndex > -1) {
            // Set repo to unregister deleted object
            message._qsRepo = null;

            // We can not destroy spliced object because it will cause a crash
            device.messages.splice(msgIndex, 1);
            device.messagesChanged();
        }
    }

    /* Property Declarations
     * ****************************************************************************************/

    // To add system alerts into messages.
    property Connections systemConnections: Connections {
        target: deviceController.system

        function onAlert(message: string) {
            addNewMessageFromData(Message.Type.SystemNotification, message, DateTimeManager.nowUTC());
        }

        function onLogAlert(message: string) {
            addNewMessageFromData(Message.Type.SystemNotification, message, DateTimeManager.nowUTC());
        }

        //! Manage update notifications (a message type)
        function onUpdateAvailableChanged() {
            // hasUpdateNotification is a UiSession property, update when updateAvailableChanged
            uiSession.hasUpdateNotification = deviceController.system.updateAvailable;
        }
    }

    property Connections syncConnections: Connections {
        target: deviceController.sync

        function onAlertPushed(alertUid: string, success: bool, alert: var) {
            if (success) {
                var pushedAlert = device.messages.find(elem => elem._qsUuid === alertUid);
                if (pushedAlert) {
                    console.log("Alert: alert pushed with id: ", alert.alert_id)
                    pushedAlert.id = alert.alert_id;
                    deviceController.saveSettings();
                }

                var alertId = sendingAlertsToServer.findIndex(elem => elem._qsUuid === alertUid);
                if (alertId !== -1) {
                    sendingAlertsToServer.splice(alertId, 1);
                    sendingAlertsToServerChanged();
                }
            }

            if (sendingAlertsToServer.length > 0) {
                pushAlertToServerTimer.start();
            }
        }
    }

    property Connections wifiConnection: Connections {
        target: NetworkInterface

        function onHasInternetChanged() {
            //! If wifi is connected, internet will be check after one minute.
            if (NetworkInterface.connectedWifi)
                checkInternetTimer.restart();

            if (NetworkInterface.hasInternet) {
                // Close the alert
                closeWifiInternetAlert();

                if (sendingAlertsToServer.length > 0) {
                    pushAlertToServerTimer.start();
                }

            } else {
                pushAlertToServerTimer.stop();
            }
        }

        function onConnectedWifiChanged() {
            checkInternetTimer.restart();
        }

        //! wrong password alert.
        function onIncorrectWifiPassword() {
            if (!device.setting.enabledAlerts && is_control_alert_feature_enable)
                return;

            var message = "Wrong password, please try again.";
            showWifiInternetAlert(message, (new Date()).toLocaleString());

            // After password is wrong, Wifi and internet check afetr one minute.
            // checkInternetTimer.restart(); // disabled for now!
        }
    }

    //! Check air quility
    property Connections airConditionWatcherCon: Connections {
        target: device
        enabled: deviceController.airConditionSensorHealth

        function onCo2Changed() {
            if (device.co2 > AppSpec.airQualityAlertThreshold) {
                airConditionWatcher.start();

            } else {
                airConditionWatcher.stop();
            }
        }
    }

    property Timer airConditionWatcher: Timer {
        repeat: false
        running: false

        interval: 3 * 60 * 60 * 1000

        onTriggered: {
            if (!deviceController.airConditionSensorHealth)
                return;

            var message = AppSpec.alertTypeToMessage(AppSpec.Alert_Air_Quality);

            console.log("Air condition alert ", message)
            if (messagesShowing.find(element => message === element.message))
                return;

            var now = (new Date()).getTime();
            if (Object.keys(lastRead).includes(message) &&
                    (now - lastRead[message]) < weeklyAlertInterval)
                return;

            addNewMessageFromData(Message.Type.Alert, message, DateTimeManager.nowUTC());
        }
    }

    property Connections  deviceControllerConnection: Connections {
        target: deviceController.deviceControllerCPP

        function onAlert(alertLevel : int,
                         alertType : int,
                         alertMessage : string) {

            console.log("Alert: ", alertLevel, alertType, alertMessage);

            var retriggerInterval = alertInterval;
            if (messagesShowing.find(element => alertMessage === element.message))
                return;

            var messageType = Message.Type.Alert;

            //! Watch some sensor alerts
            switch (alertType) {
            case AppSpec.Alert_humidity_high:
            case AppSpec.Alert_humidity_low:
            case AppSpec.Alert_temp_low:
            case AppSpec.Alert_temp_high: {
                messageType = Message.Type.SystemAlert;
                retriggerInterval = weeklyAlertInterval;

            } break;

            case AppSpec.Alert_No_Data_Received:
            case AppSpec.Alert_temperature_humidity_malfunction: {
                messageType = Message.Type.SystemAlert;
                retriggerInterval = sixHoursAlertInterval;
            } break;

            case AppSpec.Alert_iaq_high:
            case AppSpec.Alert_iaq_low:
            case AppSpec.Alert_c02_low: {
                messageType = Message.Type.SystemAlert;
                retriggerInterval = weeklyAlertInterval;

            } break;

            case AppSpec.Alert_fan_High:
            case AppSpec.Alert_fan_low: {
                // Emit this alert when device is not in night/quiet mode
                if (device.nightMode._running) {
                    return;
                }

                //! Turn on the night mode
                device.nightMode.mode =  AppSpec.NMOn;

                // TODO: The fan speed is wrong in the main data.
                // Return temporary
                return;

                messageType = Message.Type.SystemAlert;
                retriggerInterval = dailyAlertInterval;

            } break;

            case AppSpec.Alert_Light_High:
            case AppSpec.Alert_Light_Low: {
                messageType = Message.Type.SystemAlert;
                retriggerInterval = weeklyAlertInterval;

                // Disable the adaptive brightness.
                device.setting.adaptiveBrightness = false;

                // TODO: adaptive brightness is not available for now
                console.log("ignored Alert_Light alert: ++++++++++++++++++++++++++++++");
                return;

            } break;

            case AppSpec.Alert_Efficiency_Issue: {
                messageType = Message.Type.SystemAlert;
                retriggerInterval = weeklyAlertInterval;

                // TODO: outdoor can have conflict, disabled for now
                console.log("ignored Alert_Efficiency_Issue alert: ++++++++++++++++++++++++++++++");
                return;
            } break;

            default:
                break;
            }

            var now = (new Date()).getTime();

            // Check message time interval
            if (Object.keys(lastRead).includes(alertMessage) &&
                    (now - lastRead[alertMessage]) < retriggerInterval)
                return;

            addNewMessageFromData(messageType, alertMessage, DateTimeManager.nowUTC());

        }
    }

    property Connections  messagesDevice: Connections {
        target: device

        function onMessagesChanged() {
            checkUnreadMessages();
        }
    }

    //! To update unread messages and alerts when settings changed.
    property Connections  settingDevice: Connections {
        target: device.setting

        function onEnabledNotificationsChanged() {
            existUnreadMessages();
        }

        function onEnabledAlertsChanged() {
            existUnreadAlerts();
        }
    }


    /* Functions
     * ****************************************************************************************/

    function checkWifiConnection() : bool {
        //! No need to check the Wi-Fi
        if (deviceController.initialSetupNoWIFI) {
            return true;
        }

        var connectedWifi = NetworkInterface.connectedWifi;

        // Wifi message type is SystemNotification, so related to alerts
        if ((device.setting.enabledAlerts || !is_control_alert_feature_enable) && !connectedWifi) {
            var message = AppSpec.noWIFIErrorString
            showWifiInternetAlert(message, (new Date()).toLocaleString());
        }

        return connectedWifi;
    }

    function checkInternetConnection() : bool {
        //! No need to check the internet
        if (deviceController.initialSetupNoWIFI) {
            return true;
        }

        var hasInternet = NetworkInterface.hasInternet;

        // Wifi message type is SystemNotification, so related to alerts
        if ((device.setting.enabledAlerts || !is_control_alert_feature_enable) && !hasInternet) {
            var message = AppSpec.noInternetErrorString;
            showWifiInternetAlert(message, (new Date()).toLocaleString());
        }

        return hasInternet;
    }

    function addShowingMessage(message: Message) {
        messagesShowing.push(message);
        messagesShowingChanged();

        checkUnreadMessages();
    }

    function removeShowingMessage(message: Message) {
        //! Remove when the alert closed by user.
        var msgIndex = messagesShowing.findIndex((element, index) => element.message === message.message);
        if (msgIndex > -1) {
            //! Remove from messages shown
            messagesShowing.splice(msgIndex, 1);
            messagesShowingChanged();
        }

        checkUnreadMessages();

        // Update time
        var now = (new Date()).getTime();
        lastRead[message.message] = now;
    }

    //! Check unread messages and Update the uiSession parameters (hasUnreadAlerts and hasUnreadMessages)
    function checkUnreadMessages() {
        existUnreadAlerts();
        existUnreadMessages();
    }

    //! Exist unread alerts?
    function existUnreadAlerts() {
        if (!device.setting.enabledAlerts && is_control_alert_feature_enable) {
            uiSession.hasUnreadAlerts = false;
            return;
        }

        // Check unread messages
        var msgAlertIndex = device.messages.findIndex((element, index) => (element.type === Message.Type.Alert ||
                                                                           element.type === Message.Type.SystemAlert ||
                                                                           element.type === Message.Type.SystemNotification) && !element.isRead);

        uiSession.hasUnreadAlerts = msgAlertIndex > -1;

        // Wifi alerts (and maybe another types in future) which not yet supported in the model.
        if (!uiSession.hasUnreadAlerts) {
            msgAlertIndex = messagesShowing.findIndex((element, index) => (element.type === Message.Type.Alert ||
                                                                           element.type === Message.Type.SystemAlert ||
                                                                           element.type === Message.Type.SystemNotification));

            uiSession.hasUnreadAlerts = msgAlertIndex > -1;
        }
    }

    //! Exist unread messages?
    function existUnreadMessages() {

        if (!device.setting.enabledNotifications) {
            uiSession.hasUnreadMessages = false;
            return;
        }

        // Check notifications
        var msgMessageIndex = device.messages.findIndex((element, index) => (element.type === Message.Type.Notification) && !element.isRead);
        uiSession.hasUnreadMessages = msgMessageIndex > -1;

        //! To manage the messages that not exist in the model
        if (!uiSession.hasUnreadMessages) {
            msgMessageIndex = messagesShowing.findIndex((element, index) => (element.type === Message.Type.Notification));
            uiSession.hasUnreadMessages = msgMessageIndex > -1;
        }
    }

    //! Check messages to push to the server
    function checkMessagesForServer(message: Message) {
        //! Ignore the server messages/alerts.
        if (message.sourceType === Message.SourceType.Server) {
            return;
        }

        //! Send alerts to the server.
        if (message.type === Message.Type.Alert || message.type === Message.Type.SystemAlert) {
            sendingAlertsToServer.push(message);
            sendingAlertsToServerChanged();
        }

        if (!pushAlertToServerTimer.running)
            pushAlertToServerTimer.start();
    }

    function sendAlertToServer() {
        if (sendingAlertsToServer.length === 0 || !NetworkInterface.hasInternet) {
            return;
        }

        // Send first alert to server.
        var fisrtAlert = sendingAlertsToServer[0];
        var alertType = AppSpec.messageToAlertType(fisrtAlert.message);
        var alertTypeString = AppSpec.alertTypeToString(alertType);

        var sendData = {
            "alerts": [
                {
                    "type": alertTypeString
                }
            ]
        };

         deviceController.sync.pushAlertToServer(fisrtAlert._qsUuid , sendData);
    }
}
