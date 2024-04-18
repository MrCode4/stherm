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
    //! UiSession
    property UiSession          uiSession

    //! MessageController
    property MessageController  messageController

    /* Children
     * ****************************************************************************************/
    Connections {
        target: messageController
        enabled: Boolean(uiSession)

        function onNewMessageReceived(message: Message) {
            showMessagePopup(message);
        }

        function onShowMessage(message: Message) {
            showMessagePopup(message);
        }
    }

    Component {
        id: _messagePopupCompo

        AlertNotifPopup {
            onClosed: {
                message.isRead = true;
                uiSession.deviceController.pushSettings();

                destroy(this);
            }
        }
    }

    /* Functions
     * ****************************************************************************************/
    //! Show message popups
    function showMessagePopup(message: Message) {
        //! \todo This will later be shown using PopUpLayout to be able to show multiple message
        //! popups on top of each other.

        //! Create an instance of AlertNotifPopup
        var newAlertPopup = _messagePopupCompo.createObject(_root, {
                                                                "message": message
                                                            });

        if (newAlertPopup) {
            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(newAlertPopup, false);
        }
    }
}
