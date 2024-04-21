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
    property I_Device device

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

            visible: device.schedules.filter(sch => sch.enable).length > 0
            font.weight: 400
            font.pointSize: Style.fontIconSize.smallPt
            color: "#3495eb"
            text: AppStyle._menuIcons.check
        }
    }
}
