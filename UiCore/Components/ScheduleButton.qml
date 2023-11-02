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

    /* Object properties
     * ****************************************************************************************/
    contentItem: RoniaTextIcon {
        font.pointSize: Style.fontIconSize.largePt
        color: _root.Material.foreground
        text: "\uf073"
    }
}
