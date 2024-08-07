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
        }
    }

    //! Check wifi/internet after model is loaded
    //! And start it when wifi or Internet disconnected
    property Timer checkInternetTimer: Timer {
        running: device
        repeat: false

        interval: 1 * 60 * 1000

        onTriggered: {
            if (checkWifiConnection())
                if(checkInternetConnection())
                    closeWifiInternetAlert();
        }
    }

    Component.onCompleted: {
        // Filter out null/undefined messages from file data.
        device.messages = device.messages.filter(message => message !== null && message !== undefined);
        device.messagesChanged()
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
                             if (message.message === "")
                             return;

                             // we need this for now as server sents every message only once, so it should be false so user don't lose it
                             message.isRead = false;

                             // Find Schedule in the model
                             var foundMessage = device.messages.find(messageModel => (message.message === messageModel.message &&
                                                                                      messageModel.sourceType === Message.SourceType.Server));

                             var type = (message.type === Message.Type.SystemNotification) ? Message.Type.Notification : message.type;
                             var messageDatetime = message.created === null ? "" : message.created;
                             if (foundMessage && foundMessage.datetime === messageDatetime &&
                                 foundMessage.type === type) {
                                 // isRead in the server is wrong. So I use the isRead condition from the local.
                                 // foundMessage.isRead = message.isRead;

                             } else { // new message, TODO: Check empty message
                                 let icon = (message.icon === null) ? "" : message.icon;
                                 addNewMessageFromData(type, message.message, message.created, message.isRead, icon, Message.SourceType.Server);
                             }
                         });
    }

    function addNewMessageFromData(type, message, datetime, isRead = false, icon = "", sourceType = Message.SourceType.Device)
    {
        if (message.length === 0) {
            console.log("addNewMessageFromData: The message is empty!")
            return;
        }

        if (!activeAlerts) {
            console.log("ignored message: ______________________________________\n", "type : ", type, ",message:", message, "\n----------------------------------------------");
            return;
        }

        // notifyUser: Save message to model and hide from the user's view
        var notifyUser = true;

        var modifiedIsRead = isRead;

        // Check Alerts with enabledAlerts in settings
        if (!device.setting.enabledAlerts && (type === Message.Type.SystemNotification ||
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
        newMessage.datetime = datetime;
        newMessage.isRead = modifiedIsRead;
        newMessage.sourceType = sourceType;

        if (type !== Message.Type.SystemNotification) {
            device.messages.unshift(newMessage);
            device.messagesChanged();

            // Send messages to server
            deviceController.updateEditMode(AppSpec.EMMessages);
            deviceController.pushSettings();
        }

        AppCore.defaultRepo.saveToFile(uiSession.configFilePath);

        if (notifyUser)
            newMessageReceived(newMessage);
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
            // We can not destroy spliced object because it will cause a crash
            device.messages.splice(msgIndex, 1);
            device.messagesChanged();
        }
    }

    /* Property Declarations
     * ****************************************************************************************/

    // To add system alerts into messages.
    property Connections sytemConnections: Connections {
        target: deviceController.deviceControllerCPP.system

        function onAlert(message: string) {
            addNewMessageFromData(Message.Type.SystemNotification, message, (new Date()).toLocaleString());
        }

        //! Manage update notifications (a message type)
        function onUpdateAvailableChanged() {
            // hasUpdateNotification is a UiSession property, update when updateAvailableChanged
            uiSession.hasUpdateNotification = deviceController.deviceControllerCPP.system.updateAvailable;
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
            }
        }

        function onConnectedWifiChanged() {
            checkInternetTimer.restart();
        }

        //! wrong password alert.
        function onIncorrectWifiPassword() {
            if (!device.setting.enabledAlerts)
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

            var message = "Poor air quality detected. Please ventilate the room.";

            console.log("Air condition alert ", message)
            if (messagesShowing.find(element => message === element.message))
                return;

            var now = (new Date()).getTime();
            if (Object.keys(lastRead).includes(message) &&
                    (now - lastRead[message]) < weeklyAlertInterval)
                return;

            addNewMessageFromData(Message.Type.Alert, message, (new Date()).toLocaleString());
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

            addNewMessageFromData(messageType, alertMessage, (new Date()).toLocaleString());

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

        var connectedWifi = NetworkInterface.connectedWifi;

        // Wifi message type is SystemNotification, so related to alerts
        if (device.setting.enabledAlerts && !connectedWifi) {
            var message = "No Wi-Fi connection. Please check your Wi-Fi connection.";
            showWifiInternetAlert(message, (new Date()).toLocaleString());
        }

        return connectedWifi;
    }

    function checkInternetConnection() : bool {
        var hasInternet = NetworkInterface.hasInternet;

        // Wifi message type is SystemNotification, so related to alerts
        if (device.setting.enabledAlerts && !hasInternet) {
            var message = "No internet connection. Please check your internet connection.";
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
        if (!device.setting.enabledAlerts) {
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
}
