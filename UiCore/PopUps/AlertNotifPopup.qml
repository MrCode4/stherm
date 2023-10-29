import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AlertNotifPopup provides a popup for showing and alert or notification
 * ***********************************************************************************************/
I_PopUp {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Popup Message
    property Message    message

    /* Object properties
     * ****************************************************************************************/
    title: message ? (message.type === Message.Type.Alert
                      ? "Alert" : (message.type === Message.Type.Notification
                                   ? "Notification"
                                   : "Message")
                      ) : ""


    /* Children
     * ****************************************************************************************/
    Label {
        anchors.fill: parent
        textFormat: "MarkdownText"
        text: message?.message ?? ""
        wrapMode: "Wrap"
        verticalAlignment: "AlignVCenter"
        horizontalAlignment: "AlignHCenter"
    }
}
