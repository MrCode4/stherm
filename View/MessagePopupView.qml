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
    property MessageController  messageController : uiSession.messageController

    //! Keep the show/open messages
    property var messagesShown: []

    /* Children
     * ****************************************************************************************/
    Connections {
        target: messageController

        function onNewMessageReceived(message: Message) {
            showMessagePopup(message);
        }

        function onShowMessage(message: Message) {
            showMessagePopup(message);
        }

        function onShowWifiInternetAlert(message: string, dateTime: string) {
            wifiInternetConnectionAlert.message.message = message;
            wifiInternetConnectionAlert.message.datetime = dateTime;

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(wifiInternetConnectionAlert);
        }
    }

    Component {
        id: _messagePopupCompo

        AlertNotifPopup {
            uiSession: root.uiSession

            onClosed: {
                if (messageController && message && message.type !== Message.Type.SystemNotification) {
                    message.isRead = true;
                    uiSession.deviceController.pushSettings();
                }

                //! Remove when the alert closed by user.
                var msgIndex = messagesShown.findIndex((element, index) => element === message.message);
                if (msgIndex > -1) {
                    //! Remove from messages shown
                    messagesShown.splice(msgIndex, 1);
                    messagesShownChanged();
                }

                destroy(this);
            }
        }
    }

    //! Witi and Internet connection alerts
    //! This object protected
    AlertNotifPopup {
        id: wifiInternetConnectionAlert

        uiSession: root.uiSession

        message: Message {
            type: Message.Type.SystemNotification
        }

    }

    /* Functions
     * ****************************************************************************************/
    //! Show message popups
    function showMessagePopup(message: Message) {
        //! \todo This will later be shown using PopUpLayout to be able to show multiple message
        //! popups on top of each other.
        if (!message || messagesShown.includes(message.message))
            return;

        //! Create an instance of AlertNotifPopup
        var newAlertPopup = _messagePopupCompo.createObject(root, {
                                                                "message": message
                                                            });

        if (newAlertPopup) {
            messagesShown.push(message.message);

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(newAlertPopup);
        }
    }
}
