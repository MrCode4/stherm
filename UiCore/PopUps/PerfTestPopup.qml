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
            visible: PerfTestService.state == PerfTestService.Eligible || PerfTestService.state == PerfTestService.Warmup
        }

        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 5
            visible: PerfTestService.state == PerfTestService.Eligible || PerfTestService.state == PerfTestService.Warmup

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
                color: Style.foreground
                visible: PerfTestService.startTimeLeft > 0
                text: "Cooling will start in\n" + PerfTestService.startTimeLeft + " sec."
            }
        }

        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20
            visible: PerfTestService.state == PerfTestService.Running

            Item {width: 1; height: 25}

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "Performance check is in progress"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "Remaining time " + Math.ceil(PerfTestService.testTimeLeft.toFixed()/60) + " minutes"
            }

            Item {width: 1; height: 10}
        }

        Column {
            Layout.alignment: Qt.AlignHCenter
            spacing: 20
            visible: PerfTestService.state == PerfTestService.Complete

            Item {width: 1; height: 25}

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "The check is complete, and the results have\nbeen sent to your contractor."
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "You will be contacted if there is any\npotential risk related to your HVAC."
            }

            Item {width: 1; height: 10}
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
                text:  PerfTestService.state == PerfTestService.Complete ? "Close" : "Stop"
                font.bold: true
                onClicked: {
                    if (PerfTestService.state == PerfTestService.Complete) {
                        PerfTestService.finishTest();
                    }
                    else {
                        root.showConfirmationToStop = !root.showConfirmationToStop;
                        if (!root.showConfirmationToStop) {
                            PerfTestService.cancelTest();
                        }
                    }
                }
            }
        }
    }
}
