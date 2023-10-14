import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import Stherm

/*! ***********************************************************************************************
 *
 * ***********************************************************************************************/
ToolButton {
    id: _root
    padding: 12
    contentItem: RoniaTextIcon {
        color: _root.Material.foreground
        text: "\uf0c9"
    }
}
