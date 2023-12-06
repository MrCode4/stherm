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

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: _coolingStateItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: implicitWidth
    padding: 10

    /* Children
     * ****************************************************************************************/
    Item {
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
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }
        }
    }

    FontMetrics {
        id: _sizeMetric
    }

    state: {
        switch(device?.systemMode) {
        case AppSpec.SystemMode.Off:
            return "off";
        case AppSpec.SystemMode.Heating:
            return "heating";
        case AppSpec.SystemMode.Cooling:
            return "cooling";
        case AppSpec.SystemMode.Vacation:
            // there is no design for vacation, so we show it as auto
            // if design added the order should be specified as well as the next state in onClicked
        case AppSpec.SystemMode.Auto:
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
