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

    /* Children
     * ****************************************************************************************/
    Label {
        anchors.centerIn: parent
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

    //! Wait for a senor to be paired, when it is push next pages to StackView
    function startPairing()
    {
        //! Use DeviceController to start pairing
    }
}
