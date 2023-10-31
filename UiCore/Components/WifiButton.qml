import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiButton provide a ui for connecting to a wifi network and show status of current connection
 * ***********************************************************************************************/
ToolButton {
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
    padding: 8
    contentItem: RoniaTextIcon {
        font.pointSize: Style.fontIconSize.largePt - 1
        color: _root.Material.foreground
        text: wifi?.connectedSsid ? "\uf1eb" : "\uf6ac"
    }

    Component.onCompleted: background.square = true
}
