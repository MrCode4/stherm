import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * MenuButton
 * ***********************************************************************************************/
ToolButton {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Use to show a red info circle on menu
    property bool hasNotification: false

    /* Object properties
     * ****************************************************************************************/
    touchMargin: 30

    contentItem: Item {

        //! Red box
        Rectangle {
            id: notificationRect

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top

            visible: hasNotification
            width: 10
            height: 10
            radius: 5
            color: "red"
        }

        RoniaTextIcon {
            anchors.centerIn: parent
            font.pointSize: Style.fontIconSize.largePt
            color: root.Material.foreground
            text: "\uf0c9"
        }
    }
}
