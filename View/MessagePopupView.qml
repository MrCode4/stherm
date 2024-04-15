import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * MessagePopupView listens to MessageController and opens popup when a new message arrives.
 * ***********************************************************************************************/
Item {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! UiSession
    property UiSession          uiSession

    //! MessageController
    property MessageController  messageController

    /* Children
     * ****************************************************************************************/
    Connections {
        target: messageController
        enabled: Boolean(uiSession)

        function onNewMessageReceived(message)
        {
            //! \todo This will later be shown using PopUpLayout to be able to show multiple message
            //! popups on top of each other.

            //! Create an instance of AlertNotifPopup
            var newAlertPopup = _messagePopupCompo.createObject(root, {
                                                                    "message": message
                                                                });

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(newAlertPopup, false);
        }
    }

    Component {
        id: _messagePopupCompo

        AlertNotifPopup {
            uiSession: root.uiSession

            onClosed: {
                if (message.type === Message.Type.SystemNotification) {
                    messageController.removeMessage(message);
                }

                destroy(this);
            }
        }
    }
}
