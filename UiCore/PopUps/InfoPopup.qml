import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InfoPopup prompts user to confirm the test process.
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal accepted()

    /* Property declaration
     * ****************************************************************************************/
    //! Text message
    property string message: "Information"

    //! Detail
    property string detailMessage: ""


    /* Object properties
     * ****************************************************************************************/
    title: ""
    closeButtonEnabled: false
    closePolicy: Popup.NoAutoClose

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.fillWidth: true
            font.bold: true
            text: message
            wrapMode: "Wrap"
            horizontalAlignment: "AlignHCenter"
        }

        Label {
            Layout.fillWidth: true
            visible: detailMessage.length > 0
            font.italic: true
            textFormat: "RichText"
            text: `<small>${detailMessage}</small>`
            horizontalAlignment: "AlignHCenter"
        }

        Button {
            Layout.fillWidth: true
            text: qsTr("OK")

            onClicked: {
                accepted();
                close();
            }
        }
    }
}
