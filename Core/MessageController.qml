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
}
