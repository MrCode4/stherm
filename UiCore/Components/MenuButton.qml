import QtQuick

import Ronia
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
        font.pointSize: Qt.application.font.pointSize * 1.4
        color: _root.Material.foreground
        text: "\uf0c9"
    }

    onClicked: {
        //! Open menu popup
    }
}
