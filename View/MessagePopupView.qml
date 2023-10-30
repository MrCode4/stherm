import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * MessagePopupView listens to MessageController and opens popup when a new message arrives.
 * ***********************************************************************************************/
Item {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! MessageController
    required property  MessageController    messageController

    /* Children
     * ****************************************************************************************/
    Connections {
        target: messageController

        function onNewMessageReceived(message)
        {
            //! \todo This will later be shown using PopUpLayout to be able to show multiple message
            //! popups on top of each other.

            //! Display this message
            _messagePopup.message = message;
            _messagePopup.open();
        }
    }

    AlertNotifPopup {
        id: _messagePopup
    }
}
