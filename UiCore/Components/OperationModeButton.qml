import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ControlModeButton provides a ui for switching application operation modes
 * ***********************************************************************************************/
ToolButton {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! This should be moved to related controller
    enum OperationMode {
        Off,
        Heating,
        Cooling,
        Auto
    }

    property int operationMode: 0

    readonly property var _stateNames: ["off", "heating", "cooling", "auto"]

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 120
    implicitHeight: 90

    /* Children
     * ****************************************************************************************/
    Item {
        anchors.fill: parent

        //! Label for OFF state
        Label {
            id: _offStateItem
            anchors.centerIn: parent
            text: "OFF"
            visible: opacity > 0
            opacity: _control.state === "off" ? 1. : 0.
        }

        //! Item for HEATING state
        ColumnLayout {
            id: _heatingStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "heating" ? 1. : 0.

            //! HEATING mode icon
            Label {
                Layout.alignment: Qt.AlignCenter
                text: " - "
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Heating"
            }
        }

        //! Item for COOLING state
        ColumnLayout {
            id: _coolingStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "cooling" ? 1. : 0.

            //! COOLING mode icon
            Label {
                Layout.alignment: Qt.AlignCenter
                text: " - "
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Cooling"
            }
        }

        //! Item for AUTO state
        ColumnLayout {
            id: _autoStateItem
            anchors.centerIn: parent
            visible: opacity > 0
            opacity: _control.state === "auto" ? 1. : 0.

            //! AUTO mode icon
            Label {
                Layout.alignment: Qt.AlignCenter
                text: " - "
            }

            Label {
                Layout.alignment: Qt.AlignCenter
                text: "Auto"
            }
        }
    }

    state: _stateNames[operationMode] ?? ""
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
        operationMode++;
        if (operationMode > 3) {
            operationMode = 0;
        }
    }
}
