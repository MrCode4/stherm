import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleButton
 * ***********************************************************************************************/
ToolButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to UiSession
    property UiSession              uiSession

    /* Object properties
     * ****************************************************************************************/
    touchMargin: 30

    contentItem: RoniaTextIcon {
        font.pointSize: Style.fontIconSize.largePt
        color: _root.Material.foreground
        text: "\uf073"

        Rectangle {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 2

            visible: uiSession?.schedulesController?.runningScheduleEnabled ?? false
            color: white
            width: parent.width / 2 - 2
            height: width
            radius: width / 2

            RoniaTextIcon {
                anchors.centerIn: parent

                font.weight: 600
                font.pointSize: Style.fontIconSize.smallPt
                color: "#3495eb"
                text: AppStyle._menuIcons.check
            }
        }
    }
}
