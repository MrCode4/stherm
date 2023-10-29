import QtQuick
import QtQuick.Layouts

import Ronia
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
    implicitWidth: _coolingStateItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: _coolingStateItem.implicitHeight + topPadding + bottomPadding

    /* Children
     * ****************************************************************************************/
    Item {
        anchors.fill: parent

        //! Label for OFF state
        Label {
            id: _offStateItem
            anchors.centerIn: parent
            text: "\u23fbFF"
            visible: opacity > 0
            opacity: _control.state === "off" ? 1. : 0.

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
