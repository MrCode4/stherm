import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

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
    padding: 12
    contentItem: RoniaTextIcon {
        color: _root.Material.foreground
        text: "\uf073"
    }
}
