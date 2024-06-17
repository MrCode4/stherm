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
    property var messagesShowing: []

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

            messagesShowing.push(wifiInternetConnectionAlert.message);
            messagesShowingChanged();
        }

        //! Close wifi alert
        function onCloseWifiInternetAlert() {
            if (wifiInternetConnectionAlert.visible)
                wifiInternetConnectionAlert.close();

            //! Remove from messages shown
            var msgIndex = messagesShowing.findIndex((element, index) => element === wifiInternetConnectionAlert.message);
            if (msgIndex > -1) {
                messagesShowing.splice(msgIndex, 1);
                messagesShowingChanged();
            }
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
                var msgIndex = messagesShowing.findIndex((element, index) => element.message === message.message);
                if (msgIndex > -1) {
                    //! Remove from messages shown
                    messagesShowing.splice(msgIndex, 1);
                    messagesShowingChanged();
                }

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

        onOpened: {
            checkUnreadMessages();
        }

        onClosed: {
            messageController.closeWifiInternetAlert();
            checkUnreadMessages();
        }
    }

    //! TODO: Move to messsage controller
    property Connections  messagesDevice: Connections {
        target: uiSession.appModel

        function onMessagesChanged() {
            checkUnreadMessages();
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

        var msgIndex = messagesShowing.findIndex((element, index) => element.message === message.message);
        if (msgIndex > -1) {
            return;
        }

        //! Create an instance of AlertNotifPopup
        var newAlertPopup = _messagePopupCompo.createObject(root, {
                                                                "message": message
                                                            });

        if (newAlertPopup) {
            messagesShowing.push(message);
            messagesShowingChanged();

            //! Ask PopUpLayout to open popup
            uiSession.popupLayout.displayPopUp(newAlertPopup);
        }
    }


    //! Check unread messages and Update the uisseion parameters
    //! TODO: move to messageController
    function checkUnreadMessages() {
        existUnreadAlerts();
        existUnreadMessages();
    }

    //! Exist unread alerts?
    function existUnreadAlerts() {
        if (!uiSession.appModel.setting.enabledAlerts) {
            uiSession.hasUnreadAlerts = false;
            return;
        }

        // Check unread messages
        var msgAlertIndex = uiSession.appModel.messages.findIndex((element, index) => (element.type === Message.Type.Alert ||
                                                                           element.type === Message.Type.SystemAlert ||
                                                                           element.type === Message.Type.SystemNotification) && !element.isRead);

        uiSession.hasUnreadAlerts = msgAlertIndex > -1;

        // Wifi alerts (and maybe another types in future) are not yet supported in the model.
        if (!uiSession.hasUnreadAlerts) {
            msgAlertIndex = messagesShowing.findIndex((element, index) => (element.type === Message.Type.Alert ||
                                                                           element.type === Message.Type.SystemAlert ||
                                                                           element.type === Message.Type.SystemNotification));

            uiSession.hasUnreadAlerts = msgAlertIndex > -1;
        }
    }

    //! Exist unread messages?
    function existUnreadMessages() {

        if (!uiSession.appModel.setting.enabledNotifications) {
            uiSession.hasUnreadMessages = false;
            return;
        }

        // Check notifications
        var msgMessageIndex = uiSession.appModel.messages.findIndex((element, index) => (element.type === Message.Type.Notification) && !element.isRead);
        uiSession.hasUnreadMessages = msgMessageIndex > -1;

        //! To manage the messages that not exist in the model
        if (! uiSession.hasUnreadMessages) {
            msgMessageIndex = messagesShowing.findIndex((element, index) => (element.type === Message.Type.Notification));
            uiSession.hasUnreadMessages = msgMessageIndex > -1;
        }
    }
}
