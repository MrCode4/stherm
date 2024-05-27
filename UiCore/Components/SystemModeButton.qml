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

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: Math.max(56, _coolingStateItem.implicitWidth) + leftPadding + rightPadding
    implicitHeight: implicitWidth
    padding: 10

    /* Children
     * ****************************************************************************************/
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
            opacity: _control.state === "heating" ? 1. : 0.

            //! HEATING mode icon
            Image {
                Layout.alignment: Qt.AlignCenter
                source: "qrc:/Stherm/Images/sun.png"
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: Application.font.pointSize * 0.75
                text: "Heating"
                visible: !showCountdownLabel
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        //! Item for COOLING state
        ColumnLayout {
            id: _coolingStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "cooling" ? 1. : 0.

            //! COOLING mode icon
            Image {
                Layout.alignment: Qt.AlignCenter
                source: "qrc:/Stherm/Images/cool.png"
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: Application.font.pointSize * 0.75
                text: "Cooling"
                visible: !showCountdownLabel
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        //! Item for AUTO state
        ColumnLayout {
            id: _autoStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "auto" ? 1. : 0.

            //! AUTO mode icon
            Image {
                Layout.alignment: Qt.AlignCenter
                source: "qrc:/Stherm/Images/auto.png"
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                font.pointSize: Application.font.pointSize * 0.75
                text: "Auto"
                visible: !showCountdownLabel
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }

        Label {
            id: countdownLabel

            anchors.bottom: mainItem.bottom
            anchors.bottomMargin: -10
            anchors.horizontalCenter: parent.horizontalCenter

            font.pointSize: Application.font.pointSize * 0.65
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideMiddle
            visible: opacity > 0
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
        if (deviceController.currentSchedule)
            return "auto";

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
        default:
            return "off"
        }
    }
    states: [
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
