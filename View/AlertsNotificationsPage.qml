import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AlertsNotificationsPage shows a list of alerts and notifications
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    title: "Alerts/Notifications"

    /* Children
     * ****************************************************************************************/
    //! A list of all the alerts/notifications of app
    ListView {
        id: _alnoListV

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        model: appModel?.messages ?? []
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

        uiSession: root.uiSession
        dim: true
        modal: true
    }
}
