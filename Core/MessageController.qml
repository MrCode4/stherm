import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * MessageController
 * ***********************************************************************************************/
QSObject {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal newMessageReceived(Message message)


    /* Property declaration
     * ****************************************************************************************/
    //! List of all the Messages
    property var    messages: []

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

        messages.push(newMessage);
        messagesChanged();

        newMessageReceived(newMessage);
    }

    function addNewMessage(message: Message)
    {
        messages.push(message);
        messagesChanged();
    }

    function removeMessage(message: Message)
    {
        var msgIndex = messages.findIndex((element, index) => element === message);
        if (msgIndex > -1) {
            var msgToRemove = messages.splice(msgIndex, 1)[0];
            msgToRemove.destroy();
        }
    }
}
