import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AlertNotificationDelegate represents alert or notification in a ListView
 * ***********************************************************************************************/
ItemDelegate {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Message for this delegate
    property Message    message

    //! index of this delegate in model/view
    property int        delegateIndex: -1

    /* Object properties
     * ****************************************************************************************/
    text: message?.type === Message.Type.Alert
          ? "Alert" : (message?.type === Message.Type.Notification ? "Notification"
                                                                   : "Message")
    contentItem: RowLayout {
        //! Icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            text: message?.type === Message.Type.Alert
                  ? "\uf071" // triangle-exclamation icon
                  : (message?.type === Message.Type.Notification
                     ? "\uf0f3" //! bell icon
                     : "")
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            text: _root.text
            elide: "ElideRight"
        }
    }
    //! Change background based on read/not-read state

    /* Children
     * ****************************************************************************************/
}
