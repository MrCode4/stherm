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
                checkInternetConnection();
        }
    }

    /* Signals
     * ****************************************************************************************/
    signal newMessageReceived(Message message)

    signal showMessage(Message message)

    //! Show wifi/Internet connection alert.
    signal showWifiInternetAlert(message: string, dateTime: string)

    /* Methods
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

                             // Find Schedule in the model
                             var foundMessage = device.messages.find(messageModel => (message.message === messageModel.message &&
                                                                                    messageModel.sourceType === Message.SourceType.Server));

                             var type = (message.type === Message.Type.SystemNotification) ? Message.Type.Notification : message.type;
                             var messageDatetime = message.datetime === null ? "" : message.datetime;
                             if (foundMessage && foundMessage.datetime === messageDatetime &&
                                 foundMessage.type === type) {
                                 // isRead in the server is wrong. So I use the isRead condition from the local.
                                 // foundMessage.isRead = message.isRead;

                             } else { // Check empty message
                                 let icon = (message.icon === null) ? "" : message.icon;
                                 addNewMessageFromData(type, message.message, message.datetime, message.isRead, icon, Message.SourceType.Server);
                             }
                         });
    }

    function addNewMessageFromData(type, message, datetime, isRead = false, icon = "", sourceType = Message.SourceType.Device)
    {
        if (!activeAlerts) {
            console.log("ignored message: ______________________________________\n", "type : ", type, ",message:", message, "\n----------------------------------------------");
            return;
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
        newMessage.isRead = isRead;
        newMessage.sourceType = sourceType;

        if (type !== Message.Type.SystemNotification) {
            device.messages.unshift(newMessage);
            device.messagesChanged();
        }

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
        }

        function onConnectedWifiChanged() {
            checkInternetTimer.restart();
        }
 
        //! wrong password alert.
        function onIncorrectWifiPassword() {
            var message = "Wrong password, please try again.";
            addNewMessageFromData(Message.Type.SystemNotification, message, (new Date()).toLocaleString());

            // After password is wrong, Wifi and internet check afetr one minute.
            checkInternetTimer.restart();
        }
    }

    //! Temperature sensor watcher (5 minutes)
    property Timer temperatureWatcher: Timer {
        interval: 15 * 60 * 1000
        repeat: false
        running: false
    }

    //! fan sensor watcher (2 hours)
    property Timer fanWatcher: Timer {
        interval: 5 * 60 * 60 * 1000
        repeat: false
        running: false
    }

    //! Humidity sensor watcher (5 minutes)
    property Timer humidityWatcher: Timer {
        interval: 15 * 60 * 1000
        repeat: false
        running: false
    }

    //! Light sensor watcher (24 hours)
    property Timer lightWatcher: Timer {
        interval: 24 * 60 * 60 * 1000
        repeat: false
        running: false
    }

    //! Check air quility
    property Connections airConditionWatcherCon: Connections {
        target: device

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
            var message = "Poor air quality detected. Please ventilate the room.";
            addNewMessageFromData(Message.Type.Alert, message, (new Date()).toLocaleString());
        }
    }

    property Connections  deviceControllerConnection: Connections {
        target: deviceController.deviceControllerCPP

        function onAlert(alertLevel : int,
                         alertType : int,
                         alertMessage : string) {

            console.log("Alert: ", alertLevel, alertType, alertMessage);

            //! Watch some sensor alerts
            switch (alertType) {
            case AppSpec.Alert_temp_low:
            case AppSpec.Alert_temp_high: {
                if (temperatureWatcher.running)
                    return;

                temperatureWatcher.start();

            } break;

            case AppSpec.Alert_humidity_high:
            case AppSpec.Alert_humidity_low: {
                if (humidityWatcher.running)
                    return;

                humidityWatcher.start();

            } break;

            case AppSpec.Alert_fan_High:
            case AppSpec.Alert_fan_low: {
                // Return temporary
                return;
                if (fanWatcher.running)
                    return;

                fanWatcher.start();

            } break;

            case AppSpec.Alert_Light_High:
            case AppSpec.Alert_Light_Low: {
                if (lightWatcher.running)
                    return;

                lightWatcher.start();

            } break;

            default:
                break;
            }

            addNewMessageFromData(Message.Type.Alert, alertMessage, (new Date()).toLocaleString());

        }
    }

    function checkWifiConnection() : bool {
        if (!NetworkInterface.connectedWifi) {
            var message = "No Wi-Fi connection. Please check your Wi-Fi connection.";
            showWifiInternetAlert(message, (new Date()).toLocaleString());
            return false;
        }

        return true;
    }

    function checkInternetConnection() : bool {
        if (!NetworkInterface.hasInternet) {
            var message = "No internet connection. Please check your internet connection.";
            showWifiInternetAlert(message, (new Date()).toLocaleString());
            return false;
        }
        return true;
    }
}
