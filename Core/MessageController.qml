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
    property I_Device device

    property I_DeviceController deviceController

    /* Signals
     * ****************************************************************************************/
    signal newMessageReceived(Message message)


    /* Methods
     * ****************************************************************************************/
    function addNewMessageFromData(type, message, datetime)
    {
        var newMessage = QSSerializer.createQSObject("Message", ["Stherm", "QtQuickStream"], AppCore.defaultRepo);
        newMessage._qsRepo = AppCore.defaultRepo;
        newMessage.type = type;
        newMessage.message = message;
        newMessage.datetime = datetime;
        newMessage.isRead = false;

        device.messages.push(newMessage);
        device.messagesChanged();

        newMessageReceived(newMessage);
    }

    function addNewMessage(message: Message)
    {
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
            hasUpdateNotification = deviceController.deviceControllerCPP.system.updateAvailable;
        }
    }
}
