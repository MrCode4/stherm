import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ExitConfirmPopup prompts user to confirm exiting when changes are not saved.
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal accepted()
    signal rejected()

    /* Property declaration
     * ****************************************************************************************/
    //! Text message
    property string message: "Are you sure?"

    //! Detail
    property string detailMessage: ""

    //! Accept button text
    property string acceptText: qsTr("Yes")

    //! Reject button text
    property string rejectText: qsTr("No")


    /* Object properties
     * ****************************************************************************************/
    title: ""

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
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 24
            spacing: 24

            Button {
                Layout.fillWidth: true
                text: acceptText

                onClicked: {
                    accepted();
                    close();
                }
            }

            Button {
                Layout.fillWidth: true
                text: rejectText

                onClicked: {
                    rejected();
                    close();
                }
            }
        }
    }
}
