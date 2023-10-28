import QtQuick

import QtQuickStream
import Stherm

/*! ***********************************************************************************************
 * MessageController
 * ***********************************************************************************************/
QSObject {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! List of all the Messages
    property var    messages: []

    /* Methods
     * ****************************************************************************************/
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
