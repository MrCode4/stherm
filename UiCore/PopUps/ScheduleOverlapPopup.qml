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
    width: T.Overlay.overlay?.width * 0.8
    height: T.Overlay.overlay?.height * 0.7
    bottomPadding: 12
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
            text: "This schedule is overlapping with others. Enable it?"
            wrapMode: "Wrap"
            horizontalAlignment: "AlignHCenter"
        }

        Label {
            Layout.fillWidth: true
            font.italic: true
            textFormat: "RichText"
            text: "<small>Overlapping schedules will be disabled</small>"
            wrapMode: "Wrap"
            horizontalAlignment: "AlignHCenter"
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
