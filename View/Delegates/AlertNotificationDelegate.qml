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

    property string dateTimeText: message.datetime ? (Date.fromLocaleString(message.datetime).toLocaleString(locale, " (dd MMM yyyy h:mm AP)")) :
                                                     ""
    /* Object properties
     * ****************************************************************************************/
    highlighted: !message.isRead
    text: (message.type === Message.Type.Alert || message.type === Message.Type.SystemAlert) ?
              "Alert" : (message?.type === Message.Type.Notification ? "Message"
                                                                   : "Message")
    contentItem: RowLayout {
        spacing: 6
        //! Icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            text: (message.type === Message.Type.Alert || message.type === Message.Type.SystemAlert)
                  ? "\uf071" // triangle-exclamation icon
                  : (message?.type === Message.Type.Notification
                     ? "\uf0f3" //! bell icon
                     : "")
        }

        Label {
            id: messageTypeLabel

            Layout.leftMargin: 10
            Layout.alignment: Qt.AlignCenter
            text: _root.text
            elide: "ElideRight"
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            text: _root.dateTimeText
            elide: Qt.ElideRight
            font.pixelSize: messageTypeLabel.font.pixelSize - 2
        }
    }
    //! Change background based on read/not-read state

    /* Children
     * ****************************************************************************************/
}
