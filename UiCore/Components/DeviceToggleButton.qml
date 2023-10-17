import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

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
    font.pixelSize: AppStyle.size / 20
    text: isSimulationMode ? "Simulation" : "Device"

    onClicked: {
        if (uiSession)
            uiSession.simulating = !uiSession.simulating;
    }
}
