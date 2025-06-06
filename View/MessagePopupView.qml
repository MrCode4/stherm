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

            console.log("Show wifi Internet Alert: ", message);

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

            console.log("Close wifi Internet Alert:", wifiInternetConnectionAlert.message.message);

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
                    //! Messages are not being synchronized with the server for now.
                    uiSession.deviceController.saveSettings();
                }

                // Remove message from showing messages
                messageController.removeShowingMessage(message);

                // Message Updated
                uiSession.appModel.messagesChanged();

                destroy(this);
            }
        }
    }

    //! Server message popup (Messages)
    Component {
        id: _serverMessagePopupCompo

        MessagePopup {

            onClosed: {
                if (messageController && message) {
                    message.isRead = true;
                    uiSession.deviceController.saveSettings();
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

        var newNotifPopup = null;
        if (message.sourceType === Message.SourceType.Server) {
            //! Create an instance of MessagePopup
            newNotifPopup = _serverMessagePopupCompo.createObject(root, {
                                                                      "message": message
                                                                  });

        } else {
            //! Create an instance of AlertNotifPopup
            newNotifPopup = _messagePopupCompo.createObject(root, {
                                                                "message": message
                                                            });
        }

        if (newNotifPopup) {
            messageController.addShowingMessage(message);

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(newNotifPopup);
        }
    }
}
