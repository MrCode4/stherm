import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ErrorPopup
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! text
    property string message: ""

    /* Object properties
     * ****************************************************************************************/
    title: "Success!"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.fillWidth: true
            textFormat: "MarkdownText"
            text: message
            wrapMode: "Wrap"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
        }

        Button {
            Layout.alignment: Qt.AlignCenter
            text: "Ok"
            leftPadding: 32
            rightPadding: 32
            onClicked: root.close()
        }
    }
}
