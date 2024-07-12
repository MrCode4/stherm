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
        spacing: 32

        Label {
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: root.font.pointSize * 0.8
            text: "Type a 4 digit PIN code to\n      lock the thermostat"
            elide: Text.ElideMiddle
        }

        PINItem {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
