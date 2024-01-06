import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorPairPage shows a message to user and start sensor pairing process, when one sensor is
 * paired, it emits a signal to notify oterhs
 * ***********************************************************************************************/
Control {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal sensorPaired(Sensor sensor)

    /* Property declaration
     * ****************************************************************************************/
    //! UiSession
    property UiSession              uiSession

    //! Device controller
    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    /* Object Properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size
    implicitHeight: AppStyle.size
    contentItem: Item { }

    /* Children
     * ****************************************************************************************/
    //! Next/Ok button
    ButtonInverted {
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: 12
            bottomMargin: 8
        }
        text: root.state === "pairing" ? "Cancel" : "Ok"

        onClicked: {
            if (text === "Cancel") {
                countdownTmr.stop();
            } else {
                //! Start countdown for pairing
                countdownLbl.text = "60";
                countdownTmr.start();
            }
        }
    }

    Label {
        id: messageLbl
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        width: parent.width * 0.85
        padding: 32
        text: "Please remove battery tab from the sensor or push reset button for 2 "
              + "seconds to start pairing."
        wrapMode: "WordWrap"

        background: Rectangle {
            color: "transparent"
            radius: 32
            border.width: 2
            border.color: Style.foreground
        }
    }

    Label {
        id: countdownLbl
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.right
        }
        opacity: 0.
        font {
            pointSize: 64
            family: "monospace"
        }
        padding: 20
        text: "60"
    }

    Timer {
        id: countdownTmr
        interval: 1000
        repeat: true
        onTriggered: {
            countdownLbl.text = Number(countdownLbl.text) - 1;
        }
    }

    /* States and Transitions
     * ****************************************************************************************/
    states: [
        State {
            name: "pairing"
            when: countdownTmr.running

            AnchorChanges {
                target: countdownLbl
                anchors.left: undefined
                anchors.horizontalCenter: root.contentItem.horizontalCenter
                anchors.verticalCenter: root.contentItem.verticalCenter
            }
            PropertyChanges {
                target: countdownLbl
                opacity: 1.
            }

            AnchorChanges {
                target: messageLbl
                anchors.horizontalCenter: undefined
                anchors.right: root.contentItem.left
            }
            PropertyChanges {
                target: messageLbl
                opacity: 0.
            }
        }
    ]

    transitions: [
        Transition {
            to: "pairing"
            reversible: true

            AnchorAnimation {
                targets: [countdownLbl, messageLbl]
                duration: 150
            }

            PropertyAnimation {
                targets: [countdownLbl, messageLbl]
                property: "opacity"
                duration: 150
            }
        }
    ]

    //! Wait for a senor to be paired, when it is push next pages to StackView
    function startPairing()
    {
        //! Use DeviceController to start pairing
    }
}
