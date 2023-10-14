import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

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
    icon {
        width: 32
        height: 32
    }

    /* Childrent
     * ****************************************************************************************/
    Image {
        anchors.centerIn: parent
        width: _root.icon.width
        height: _root.icon.height
        sourceSize.width: width
        sourceSize.height: height
        source: "qrc:/Stherm/Images/Wifi/disconnected.svg"
    }

    onClicked: {
        //! Open a popup to select wifi
    }
}
