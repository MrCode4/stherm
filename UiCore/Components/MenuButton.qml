import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

import Stherm

/*! ***********************************************************************************************
 *
 * ***********************************************************************************************/
RoundButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    flat: true
    padding: 12
    contentItem: RoniaTextIcon {
        font.pixelSize: 24
        color: _root.Material.foreground
        text: "\uf0c9"
    }

    onClicked: {
        //! Open menu popup
    }
}
