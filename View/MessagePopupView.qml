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

            if (wifiInternetConnectionAlert.visible)
                return;

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(wifiInternetConnectionAlert);

            messageController.addShowingMessage(wifiInternetConnectionAlert.message);
        }

        //! Close wifi alert
        function onCloseWifiInternetAlert() {
            if (wifiInternetConnectionAlert.visible)
                wifiInternetConnectionAlert.close();

            //! Remove from messages shown
            messageController.removeShowingMessage(wifiInternetConnectionAlert.message);
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

                // Remove message from showing messages
                messageController.removeShowingMessage(message);

                // Message Updated
                uiSession.appModel.messagesChanged();

                destroy(this);
            }
        }
    }

    //! Wifi and Internet connection alerts
    AlertNotifPopup {
        id: wifiInternetConnectionAlert
        objectName: "InternetPopup"

        uiSession: root.uiSession

        message: Message {
            type: Message.Type.SystemNotification
        }

        onClosed: {
            messageController.closeWifiInternetAlert();
        }
    }

    /* Functions
     * ****************************************************************************************/
    //! Show message popups
    function showMessagePopup(message: Message) {
        //! \todo This will later be shown using PopUpLayout to be able to show multiple message
        //! popups on top of each other.
        if (!message)
            return;

        var msgIndex = messageController.messagesShowing.findIndex((element, index) => element.message === message.message);
        if (msgIndex > -1) {
            return;
        }

        //! Create an instance of AlertNotifPopup
        var newAlertPopup = _messagePopupCompo.createObject(root, {
                                                                "message": message
                                                            });

        if (newAlertPopup) {
            messageController.addShowingMessage(message);

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(newAlertPopup);
        }
    }
}
