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

    property bool hasFontAwesomeIcon: true

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
            id: fontAwesomeIcon

            visible: hasFontAwesomeIcon
            Layout.preferredWidth: implicitHeight * 1.5
            text: delegateData?.icon ?? ""
        }

        Image {
            id: imageIcon

            visible: !hasFontAwesomeIcon
            fillMode: Image.PreserveAspectFit

            source: hasFontAwesomeIcon ? "" : (delegateData?.icon ?? "")
        }

        //! Notification rectangle
        Rectangle {
            id: notificationRect

            parent: hasFontAwesomeIcon ? fontAwesomeIcon : imageIcon
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: -10

            visible: root.hasNotification
            width: 10
            height: 10
            radius: 5
            color: "red"
        }

        Label {
            Layout.fillWidth: true
            text: root.text
            verticalAlignment: "AlignVCenter"
        }
    }
}
