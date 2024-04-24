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

        RoniaTextIcon {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 5

            visible: uiSession?.schedulesController?.runningScheduleEnabled ?? false
            font.weight: 400
            font.pointSize: Style.fontIconSize.smallPt
            color: "#3495eb"
            text: AppStyle._menuIcons.check
        }
    }
}
