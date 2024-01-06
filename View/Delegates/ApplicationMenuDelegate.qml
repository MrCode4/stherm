import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ApplicationMenuDelegate is a simple delegate for ApplicationMenuList
 * ***********************************************************************************************/
ItemDelegate {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Use to show a red info circle on menu
    property bool hasNotification: false

    //! Holds delegate data
    property var    delegateData

    //! Holds delegate index
    property int    delegateIndex

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 4 * scaleFactor
    contentItem: RowLayout {
        spacing: 10 * scaleFactor
        //! Icon: this is supposed to be a unicode of Font Awesome
        RoniaTextIcon {
            Layout.preferredWidth: implicitHeight * 1.5
            text: delegateData?.icon ?? ""

            //! Notification rectangle
            Rectangle {
                id: notificationRect

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: -10

                visible: root.hasNotification
                width: 5
                height: 5
                radius: 2
                color: "red"
            }
        }

        Label {
            Layout.fillWidth: true
            text: root.text
            verticalAlignment: "AlignVCenter"
        }
    }
}
