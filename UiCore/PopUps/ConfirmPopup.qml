import QtQuick
import QtQuick.Templates as T
import QtQuick.Layouts
import QtQuick.Dialogs

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
    signal buttonClicked(button: int)


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

    //! Support the `No`, `Yes`, `Cancel` and `Discard` for now
    //! `Cancel` and `Discard` are ButtonInverted and the others are Button type
    //! TODO: Supports two buttons, UI should be modified for more than two bottons
    property int buttons : MessageDialog.Yes | MessageDialog.No

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

                visible: (buttons & MessageDialog.Yes) === MessageDialog.Yes

                onClicked: {
                    accepted();
                    buttonClicked(MessageDialog.Yes);
                    close();
                }
            }

            Button {
                Layout.fillWidth: true
                text: rejectText
                visible: (buttons & MessageDialog.No) === MessageDialog.No

                onClicked: {
                    rejected();
                    buttonClicked(MessageDialog.No);
                    close();
                }
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "Cancel"

                visible: (buttons & MessageDialog.Cancel) === MessageDialog.Cancel

                onClicked: {
                    buttonClicked(MessageDialog.Cancel);
                    close();
                }
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "Discard"
                textColor: "#CB0C0A"
                visible: (buttons & MessageDialog.Discard) === MessageDialog.Discard

                onClicked: {
                    buttonClicked(MessageDialog.Discard);
                    close();
                }
            }
        }
    }
}
