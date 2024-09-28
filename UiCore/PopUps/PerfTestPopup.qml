import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

I_PopUp {
    id: root
    title: showConfirmationToStop ? "Stop the Performance Test" : "Performance Test"
    leftPadding: 24; rightPadding: 24; topPadding: 20; bottomPadding: 24
    closeButtonEnabled: false
    closePolicy: Popup.NoAutoClose

    property UiSession uiSession
    property I_Device appModel: uiSession?.appModel ?? null
    property bool showConfirmationToStop: false

    background: Rectangle {
        border.width: 4
        border.color: Style.foreground
        color: Style.background
    }

    ColumnLayout {
        spacing: 20

        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            font.family: "Montserrat"
            font.pixelSize: 14
            font.weight: 400
            text: "Your HVAC system needs to perform a\n15-minute system check to ensure it is ready for\nthe season."
            color: Style.foreground
        }

        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 5

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/Stherm/Images/%1".arg(PerfTestService.mode == AppSpecCPP.Cooling ? "cool.png" : "sun.png")
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                text: "Cooling will start in\n" + "155" + " sec"
                color: Style.foreground
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            ButtonInverted {
                leftPadding: 8
                rightPadding: 8
                text: "Cancel"
                font.bold: true
                visible: root.showConfirmationToStop
                onClicked: root.showConfirmationToStop = false
            }

            ButtonInverted {
                leftPadding: 8
                rightPadding: 8
                text: "Stop"
                font.bold: true
                onClicked: {
                    root.showConfirmationToStop = !root.showConfirmationToStop;
                    if (!root.showConfirmationToStop) {
                        PerfTestService.cancelTest();
                        root.close();
                    }
                }
            }
        }
    }
}
