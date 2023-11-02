import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * HoldButton is an specialized button for setting hold property of device
 * ***********************************************************************************************/
TabButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! UiSessionPopups
    property UiSessionPopups    popups

    //! Determines whether 'hold' is enabled or not
    property bool               isHoldEnabled: false

    /* Object properties
     * ****************************************************************************************/
    opacity: isHoldEnabled ? 1. : 0.6
    font.pointSize: Qt.application.font.pointSize * 1.25
    text: "Hold"

    onClicked: {
        //! Ask openning a Popup for changing hold value
    }
}
