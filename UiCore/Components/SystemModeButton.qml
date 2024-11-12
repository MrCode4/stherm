import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm
/*! ***********************************************************************************************
 * SystemModeButton provides a ui for switching application operation (system) modes
 * ***********************************************************************************************/
ToolButton {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! I_DeviceController
    property I_DeviceController     deviceController

    //! I_Device
    property I_Device               device: deviceController?.device ?? null

    property bool showCountdownLabel: systemDelayCounter > -1

    property int systemDelayCounter: -1

    property int realMode: AppSpec.Cooling

    property int dfhSystemType: deviceController.dfhSystemType

    property bool dfhTroubleshootingMode: device?.systemSetup?.systemType === AppSpec.DualFuelHeating && !NetworkInterface.hasInternet &&
                                          device.systemSetup.isAUXAuto &&
                                          (dfhSystemType === AppSpec.HeatingOnly || dfhSystemType === AppSpec.HeatPump)


    /* Object properties
     * ****************************************************************************************/
    implicitWidth: metrics.boundingRect("Cooling").width + leftPadding + rightPadding
    implicitHeight: implicitWidth
    padding: 8

    clickable: enabled
    hoverEnabled: enabled


    /* Children
     * ****************************************************************************************/
    FontMetrics {
        id: metrics
        font: coolingLbl.font
    }

    Item {
        id: mainItem

        anchors.fill: parent

        //! Label for OFF state
        Row {
            id: _offStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "off" ? 1. : 0.
            spacing: 1

            //! Power off icon
            RoniaTextIcon {
                y: (parent.height - height) / 2
                text: "\uf011" //! power-off icon
            }

            Label {
                y: (parent.height - height) / 2
                text: "FF"
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        //! Item for HEATING state
        ColumnLayout {
            id: _heatingStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: (_control.state === "heating" || _control.state === "emergency") ? 1. : 0.
            spacing: 2

            //! HEATING mode icon
            RoniaTextIcon {
                Layout.alignment: Qt.AlignCenter

                text: FAIcons.sun_bright
                font.weight: 300
                color: heatingLabel.color
            }


            Label {
                id: heatingLabel

                Layout.alignment: Qt.AlignCenter
                font.pointSize: Application.font.pointSize * 0.65
                text: {
                    if (_control.state === "emergency")
                        return "Emergency"

                    if (device.systemSetup.systemType === AppSpec.DualFuelHeating && !device.systemSetup.isAUXAuto) {

                        if (device.systemSetup.isHeatingAUX) {
                            return "Heating (Aux)"

                        } else if (dfhSystemType === AppSpec.HeatPump && !device.systemSetup.isHeatingAUX
                                   && !NetworkInterface.hasInternet) {
                            return "Heating (Heat pump)"
                        }
                    }

                    return "Heating";
                }

                color: dfhTroubleshootingMode ? AppStyle.primaryOrange : Style.foreground

                opacity: showCountdownLabel ? 0 : 1
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        //! Item for COOLING state
        ColumnLayout {
            id: _coolingStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "cooling" ? 1. : 0.
            spacing: 2

            //! COOLING mode icon
            RoniaTextIcon {
                Layout.alignment: Qt.AlignCenter
                text: FAIcons.snowflake
                font.weight: 300
                color: heatingLabel.color
            }

            Label {
                id: coolingLbl
                Layout.alignment: Qt.AlignCenter
                font.pointSize: Application.font.pointSize * 0.65
                text: "Cooling"
                opacity: showCountdownLabel ? 0 : 1
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        //! Item for AUTO state
        ColumnLayout {
            id: _autoStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "auto" ? 1. : 0.
            spacing: 2

            //! AUTO mode icon
            Image {
                Layout.alignment: Qt.AlignCenter
                source: "qrc:/Stherm/Images/auto.png"
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: Application.font.pointSize * 0.65
                text: "Auto"
                opacity: showCountdownLabel ? 0 : 1
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        Label {
            id: countdownLabel

            anchors.bottom: mainItem.bottom
            anchors.bottomMargin: -8
            anchors.horizontalCenter: parent.horizontalCenter

            font.pointSize: Application.font.pointSize * 0.65
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            visible: opacity > 0
            enabled: visible
            color: dfhTroubleshootingMode ? AppStyle.primaryOrange : Style.foreground
            opacity: showCountdownLabel ? 1. : 0.

            Behavior on opacity { NumberAnimation { duration: 200 } }

            text: {

                var text = "Cooling"

                if (realMode === AppSpec.Heating) {
                    text = "Heating";
                }

                text += " will start in\n";
                text += (systemDelayCounter + " secs");

                return text;
            }
        }
    }

    Timer {
        interval: 1000
        running: systemDelayCounter > -1
        repeat: true
        onTriggered: {
            systemDelayCounter--;
        }
    }

    Connections {
        target: deviceController.deviceControllerCPP

        function onStartSystemDelayCountdown(mode: int, delay: int) {
            realMode = mode;
            // Convert miliseconds to secs
            systemDelayCounter = delay / 1000;
        }

        function onStopSystemDelayCountdown() {
            systemDelayCounter = -1;
        }
    }

    state: {
        switch(device?.systemSetup?.systemMode) {
        case AppSpecCPP.Off:
            return "off";
        case AppSpecCPP.Heating:
            return "heating";
        case AppSpecCPP.Cooling:
            return "cooling";
        case AppSpecCPP.Vacation:
            // there is no design for vacation, so we show it as auto
            // if design added the order should be specified as well as the next state in onClicked
        case AppSpecCPP.Auto:
            return "auto";
        case AppSpecCPP.EmergencyHeat:
            return "emergency";

        default:
            return "off"
        }
    }
    states: [
        State {
            name: "schedule"
        },

        State {
            name: "off"
        },

        State {
            name: "heating"
        },

        State {
            name: "cooling"
        },

        State {
            name: "auto"
        }
    ]
}
