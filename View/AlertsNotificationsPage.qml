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
        model: []
        delegate: AlertNotificationDelegate {
            required property var modelData
            required property int index

            width: ListView.view.width
            height: Material.delegateHeight
            alnotData: modelData
            delegateIndex: index
        }
    }
}
