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
    title: "Schedule Overlap"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 8

        Label {
            Layout.fillWidth: true
            font.bold: true
            text: "This schedule is overlapping with others. Save it as Enabled?"
            wrapMode: "Wrap"
            horizontalAlignment: "AlignHCenter"
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: implicitWidth > root.width
            font.italic: true
            textFormat: "RichText"
            text: "<small>Yes: Overlapping schedules will be disabled<br>"
                  + "No: This schedule will be disabled</small>"
            wrapMode: "Wrap"
        }

        RowLayout {
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 12
            spacing: 24

            Button {
                Layout.fillWidth: true
                text: "Yes"

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
