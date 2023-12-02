import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DeviceToggleButton is an specialized button for setting hold property of device
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
    opacity: isSimulationMode ? 1 : 0
    font.pointSize: Qt.application.font.pointSize * 1.4
    text: "Simulation"

    onClicked: {
        return;
        if (uiSession)
            uiSession.simulating = !uiSession.simulating;
    }
}
