import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiButton provide a ui for connecting to a wifi network and show status of current connection
 * ***********************************************************************************************/
RoundButton {
    id: _root

    /* Property declration
     * ****************************************************************************************/
    //! Referenct to WifiController
    property WifiController     wifiController

    //! Reference to Wifi model
    property Wifi               wifi

    //! UiSessionPopups
    property UiSessionPopups    popups

    /* Object properties
     * ****************************************************************************************/
    flat: true
    padding: 12
    contentItem: RoniaTextIcon {
        font.pixelSize: 24
        color: _root.Material.foreground
        text: "\uf6ac"
    }

    onClicked: {
        //! Go to select wifi page
    }
}
