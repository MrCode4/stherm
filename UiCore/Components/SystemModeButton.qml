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
    implicitHeight: _coolingStateItem.implicitHeight + topPadding + bottomPadding

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
            spacing: 0

            //! Power off icon
            RoniaTextIcon {
                text: "\uf011" //! power-off icon
            }

            Label {
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
            return ""
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

    onClicked: {
        //! Find next state
        var nextMode = -1;

        switch(device?.systemMode) {
        case AppSpec.SystemMode.Off:
            nextMode = AppSpec.SystemMode.Heating;
            break;
        case AppSpec.SystemMode.Heating:
            nextMode = AppSpec.SystemMode.Cooling;
            break;
        case AppSpec.SystemMode.Cooling:
            nextMode = AppSpec.SystemMode.Auto;
            break;
        case AppSpec.SystemMode.Vacation:
        case AppSpec.SystemMode.Auto:
            nextMode = AppSpec.SystemMode.Off;
            break;
        }

        if (nextMode > -1) {
            deviceController.setSystemModeTo(nextMode);
        }
    }
}
