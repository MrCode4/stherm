import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AlertsNotificationsPage shows a list of alerts and notifications
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! MessageController
    property MessageController      messageController: uiSession?.messageController ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Alerts/Notifications"

    /* Children
     * ****************************************************************************************/
    //! A list of all the alerts/notifications of app
    ListView {
        id: _alnoListV

        ScrollIndicator.vertical: ScrollIndicator { }

        anchors.fill: parent
        clip: true
        model: messageController?.messages ?? 0
        delegate: AlertNotificationDelegate {
            required property var modelData
            required property int index

            width: ListView.view.width
            height: Material.delegateHeight
            message: (modelData instanceof Message ? modelData : null)
            delegateIndex: index

            onClicked: {
                //! Show Message in a popup
                _alertNotifPop.message = message;
                _alertNotifPop.open();
            }
        }
    }

    //! This should be shown using popup layout
    AlertNotifPopup {
        id: _alertNotifPop
        dim: true
        modal: true
    }
}
