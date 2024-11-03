import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import Ronia
import Stherm

I_PopUp {
    id: root
    title: PerfTestService.state != PerfTestService.Complete && root.showConfirmationToStop ? "Stop the Performance Test" : "Performance Test"
    dim: false
    leftPadding: 20; rightPadding: 20; topPadding: 16; bottomPadding: 16
    closeButtonEnabled: false
    closePolicy: Popup.NoAutoClose

    required property UiSession uiSession
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
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                        root.showConfirmationToStop = false;
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
            spacing: 5

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "Your HVAC system needs to perform a 15-minute system check to ensure it is ready for the season."
            }

            Item {width: 1; Layout.fillHeight: true}

            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                spacing: 5

                RoniaTextIcon {
                    Layout.alignment: Qt.AlignHCenter
                    text: PerfTestService.mode == AppSpecCPP.Cooling ? FAIcons.snowflake : FAIcons.sun_bright
                }

                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pointSize: Qt.application.font.pointSize * 0.7
                    color: Style.foreground
                    visible: PerfTestService.startTimeLeft > 0
                    wrapMode: Text.Wrap
                    text: (PerfTestService.mode == AppSpecCPP.Cooling ? "Cooling" : "Heating") +
                          " will start in " + PerfTestService.startTimeLeft + (PerfTestService.startTimeLeft > 1 ? " secs." : " sec.")
                }
            }

            Item {width: 1; Layout.fillHeight: true}
        }
    }

    Component {
        id: compRunning
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            spacing: 20

            Item {width: 1; Layout.fillHeight: true}

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "Performance check is in progress"
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                wrapMode: Text.Wrap
                property int remaintingTime : Math.ceil(PerfTestService.testTimeLeft.toFixed()/60)
                text: "Remaining time " + remaintingTime + (remaintingTime > 1 ? " minutes" : " minute");
            }

            Item {width: 1; Layout.fillHeight: true}
        }
    }

    Component {
        id: compComplete
        ColumnLayout {
            spacing: 20

            Item {width: 1; Layout.fillHeight: true;}
            Item {width: 1; Layout.fillHeight: true;}

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.family: "Montserrat"
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "The check is complete, and the results have been sent to your contractor."
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                wrapMode: Text.Wrap
                text: "You will be contacted if there is any potential risk related to your HVAC."
            }

            Item {width: 1; Layout.fillHeight: true}
        }
    }

    Component {
        id: compCancel
        ColumnLayout {
            spacing: 0

            Item {width: 1; Layout.fillHeight: true}

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                text: "Stopping the performance check will prevent the contractor from identifying potential issues with your HVAC system."
                wrapMode: Text.Wrap
            }

            Item {width: 1; Layout.fillHeight: true}

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                text: "Your thermostat will return to the previous mode."
                wrapMode: Text.Wrap
            }

            Item {width: 1; Layout.fillHeight: true}

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pointSize: Qt.application.font.pointSize * 0.7
                color: Style.foreground
                text: "Are you sure you want to stop?"
                wrapMode: Text.Wrap
            }

            Item {width: 1; Layout.fillHeight: true}
        }
    }

    Connections {
        target: PerfTestService
        function onIsTestRunningChanged() {
            console.log('PerfTestServiceLog: isTestRunning', PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMSchedule, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMVacation, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMRequestedHumidity, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMDesiredTemperature, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMSettings, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMSystemSetup, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMSystemMode, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMAutoMode, PerfTestService.isTestRunning);
            uiSession.deviceController.updateLockMode(AppSpec.EMMessages, PerfTestService.isTestRunning);
        }
    }
}
