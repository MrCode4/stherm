import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleButton
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
        text: "\uf073"
    }

    onClicked: {
        //! Go to schedule page
    }
}
