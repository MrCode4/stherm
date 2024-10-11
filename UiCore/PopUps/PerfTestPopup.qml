import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

I_PopUp {
    id: root
    title: PerfTestService.state != PerfTestService.Complete && root.showConfirmationToStop ? "Stop the Performance Test" : "Performance Test"
    dim: false
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

    width: AppStyle.size * 0.8
    height: AppStyle.size * 0.7

    contents: ColumnLayout {
        spacing: 20
        anchors.fill: parent

        Loader {
            id: loader
            Layout.fillWidth: true;
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            sourceComponent: {
                if (PerfTestService.state != PerfTestService.Complete && root.showConfirmationToStop) {
                    return compCancel
                }
                else {
                    switch(PerfTestService.state) {
                        case PerfTestService.Warmup:
                            return compWarmup;
                        case PerfTestService.Running:
                            return compRunning;
                        case PerfTestService.Complete:
                            return compComplete;
                    }
                }
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

            Item {
                Layout.fillWidth: true
                width: 1
                visible: root.showConfirmationToStop
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
                        if (root.showConfirmationToStop) {
                            PerfTestService.cancelTest();
                            root.showConfirmationToStop = false;
                        }
                        else {
                            root.showConfirmationToStop = true;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: compWarmup
        ColumnLayout {
            spacing: 40

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "Your HVAC system needs to perform a 15-minute system check to ensure it is ready for the season."
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                spacing: 5

                Image {
                    Layout.alignment: Qt.AlignHCenter
                    source: "qrc:/Stherm/Images/%1".arg(PerfTestService.mode == AppSpecCPP.Cooling ? "cool.png" : "sun.png")
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.family: "Montserrat"
                    font.pixelSize: 14
                    font.weight: 400
                    color: Style.foreground
                    visible: PerfTestService.startTimeLeft > 0
                    wrapMode: Text.Wrap
                    text: (PerfTestService.mode == AppSpecCPP.Cooling ? "Cooling" : "Heating") +
                          " will start in " + PerfTestService.startTimeLeft + " sec."
                }
            }
        }
    }

    Component {
        id: compRunning
        ColumnLayout {
            spacing: 20

            Item {Layout.fillWidth: true; Layout.preferredHeight: 30}

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "Performance check is in progress"
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "Remaining time " + Math.ceil((PerfTestService.testTimeLeft/60).toFixed()) + " secs"
            }
        }
    }

    Component {
        id: compComplete
        ColumnLayout {
            spacing: 20

            Item {Layout.fillWidth: true; Layout.preferredHeight: 30}

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "The check is complete, and the results have been sent to your contractor."
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "You will be contacted if there is any potential risk related to your HVAC."
            }
        }
    }

    Component {
        id: compCancel
        ColumnLayout {
            spacing: 20

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "Stopping the performance check will prevent the contractor from identifying potential issues with your HVAC system."
                wrapMode: Text.Wrap
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "Your thermostat will return to the previous mode."
                wrapMode: Text.Wrap
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pixelSize: 14
                font.weight: 400
                color: Style.foreground
                text: "Are you sure you want to stop?"
                wrapMode: Text.Wrap
            }
        }
    }
}
