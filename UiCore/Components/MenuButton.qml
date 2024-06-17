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
        //! Use an icon instead for updates.
        //! It might also be useful for other notifications in the future.
        Rectangle {
            id: notificationRect

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top

            visible: false && hasNotification
            width: 10
            height: 10
            radius: 5
            color: "red"
        }

        //! Image to show an update is available
        Image {
            id: swUpdateIcon

            anchors.left: icon.right
            anchors.bottom: icon.bottom

            visible: deviceController.deviceControllerCPP.system.updateAvailable
            fillMode: Image.PreserveAspectFit
            source: AppSpec.swUpdateIcon
            sourceSize.width: Style.fontIconSize.smallPt * 1.2
            sourceSize.height: Style.fontIconSize.smallPt * 1.2

            cache: true
        }

        RoniaTextIcon {
            id: icon

            anchors.centerIn: parent
            font.pointSize: Style.fontIconSize.largePt
            color: root.Material.foreground
            text: "\uf0c9"
        }
    }
}
