import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * LockPage
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Lock"

    //! Contents
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width
        spacing: 8

        Label {
            font.pointSize: _root.font.pointSize * 0.8
            text: "Type a 4 digit PIN code to\nlock the thermostat"
        }
    }
}
