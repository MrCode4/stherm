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

    property Timer activeAlertsTimer: Timer {
        running: true
        repeat: false

        //! Activation of alerts after 3 minutes of program start.
        interval: 3 * 1000

        onTriggered: {
            activeAlerts = true;

            // Show messages that isRead is false
            device.messages.forEach(message => {
                                    if (!message.isRead) {
                                            newMessageReceived(message);
                                        }
                                    });
        }
    }

    /* Signals
     * ****************************************************************************************/
    signal newMessageReceived(Message message)

    signal showMessage(Message message)

    /* Methods
     * ****************************************************************************************/
    function setMessagesServer(messages: var) {
        // messages uid

        if (!Array.isArray(messages)) {
            console.log("Invalid server input. Expected arrays (messages).");
            return;
        }


        let messagesModel = device.messages;

        // Delete messages from model when a cloud message is removed.
        {}

        messages.forEach(message => {
                             if (message.message === "")
                                return;

                             // Find Schedule in the model
                             var foundMessage = messagesModel.find(messageModel => (message.message === messageModel.message &&
                                                                                    messageModel.sourceType === Message.SourceType.Server));

                             var messageDatetime = message.datetime === null ? "" : message.datetime;
                             if (foundMessage && foundMessage.datetime === messageDatetime &&
                                 foundMessage.type === message.type) {
                                 // isRead in the server is wrong. So I use the isRead condition from the local.
                                 // foundMessage.isRead = message.isRead;

                             } else { // Check empty message
                                 let icon = (message.icon === null) ? "" : message.icon;
                                 addNewMessageFromData(message.type, message.message, message.datetime, message.isRead, icon, Message.SourceType.Server);
                             }
                         });
    }

    function addNewMessageFromData(type, message, datetime, isRead = false, icon = "", sourceType = Message.SourceType.Device)
    {
        if (!activeAlerts) {
            console.log("ignored message: ______________________________________\n", "type : ", type, ",message:", message, "\n----------------------------------------------");
            return;
        }

        var newMessage = QSSerializer.createQSObject("Message", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
        newMessage._qsRepo = AppCore.defaultRepo;
        newMessage.type = type;
        newMessage.message = message;
        newMessage.datetime = datetime;
        newMessage.isRead = isRead;
        newMessage.sourceType = sourceType;

        device.messages.push(newMessage);
        device.messagesChanged();

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
            var msgToRemove = device.messages.splice(msgIndex, 1)[0];
            msgToRemove.destroy();
        }
    }

    // To add system alerts into messages.
    property Connections sytemConnections: Connections {
        target: deviceController.deviceControllerCPP.system

        function onAlert(message: string) {
            addNewMessageFromData(Message.Type.Alert, message, (new Date()).toLocaleString());
        }

        //! Manage update notifications (a message type)
        function onUpdateAvailableChanged() {
            // hasUpdateNotification is a UiSession property, update when updateAvailableChanged
            uiSession.hasUpdateNotification = deviceController.deviceControllerCPP.system.updateAvailable;
        }
    }
}
