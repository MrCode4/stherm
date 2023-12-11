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

    /* Object properties
     * ****************************************************************************************/
    title: "Exit"

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
            text: "Do you want to save changes before exiting?"
            wrapMode: "Wrap"
            horizontalAlignment: "AlignHCenter"
        }

        Label {
            Layout.fillWidth: true
            font.italic: true
            textFormat: "RichText"
            text: "<small>Changes are lost if not saved.</small>"
            horizontalAlignment: "AlignHCenter"
        }

        RowLayout {
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 24
            spacing: 24

            Button {
                Layout.fillWidth: true
                text: "Save"

                onClicked: {
                    accepted();
                    close();
                }
            }

            Button {
                Layout.fillWidth: true
                text: "No"

                onClicked: {
                    rejected();
                    close();
                }
            }
        }
    }
}
