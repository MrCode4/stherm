import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * HoldButton is an specialized button for setting hold property of device
 * ***********************************************************************************************/
Button {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    property UiSession      uiSession

    //! Determines whether simulation mode is enabled or not
    property bool           isSimulationMode: uiSession?.simulating ?? true

    /* Object properties
     * ****************************************************************************************/
    flat: true
    opacity: uiSession ? 1. : 0.6
    font.pointSize: Qt.application.font.pointSize * 1.4
    text: isSimulationMode ? "Simulation" : "Device"

    onClicked: {
        if (uiSession)
            uiSession.simulating = !uiSession.simulating;
    }
}
