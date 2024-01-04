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
    contentItem: Item {
        Rectangle {
            id: notificationRect

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: -5

            visible: hasNotification
            width: 5
            height: 5
            radius: 2
            color: "red"
        }

        RoniaTextIcon {
            font.pointSize: Style.fontIconSize.largePt
            color: _root.Material.foreground
            text: "\uf0c9"
        }
    }
}
