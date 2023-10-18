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
    //! UiSessionPopups
    property UiSessionPopups    popups

    //! Determines whether 'hold' is enabled or not
    property bool               isHoldEnabled: false

    /* Object properties
     * ****************************************************************************************/
    flat: true
    opacity: isHoldEnabled ? 1. : 0.6
    font.pixelSize: AppStyle.size / 20
    text: "Hold"

    onClicked: {
        //! Ask openning a Popup for changing hold value
    }
}
