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
    //! Alert/Notification -> They better be of same type or atlease share a common base class
    property var        alnotData

    //! index of this delegate in model/view
    property int        delegateIndex: -1

    /* Object properties
     * ****************************************************************************************/
    text: alnotData?.type === "Alert" || alnotData?.type === "Notification" ? alnotData.type : "UNKNOWN"
    contentItem: RowLayout {
        //! Icon
        RoniaTextIcon {
            Layout.alignment: Qt.AlignCenter
            text: alnotData?.type === "Alert" ? "\uf071" // triangle-exclamation icon
                                              : (alnotData?.type === "Notification"
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
