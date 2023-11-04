import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiButton provide a ui for connecting to a wifi network and show status of current connection
 * ***********************************************************************************************/
ToolButton {
    id: _root

    /* Property declration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    Image {
        id: _wifiIcon
        anchors.centerIn: parent
        width: _root.width - 8
        height: width
        visible: false

        fillMode: Image.PreserveAspectFit
        smooth: true
        source: NetworkInterface.connectedWifi
                ? (NetworkInterface.connectedWifi.strength > 75
                   ? "qrc:/Stherm/Images/Wifi/wifi.png"
                   : (NetworkInterface.connectedWifi.strength > 50
                      ? "qrc:/Stherm/Images/Wifi/wifi-good.png"
                      : (NetworkInterface.connectedWifi.strength > 25
                         ? "qrc:/Stherm/Images/Wifi/wifi-fair.png"
                         : "qrc:/Stherm/Images/Wifi/wifi-weak.png"
                             )
                      )
                   )
                : "qrc:/Stherm/Images/Wifi/wifi-off.png"

    }

    ColorOverlay {
        anchors.fill: _wifiIcon
        source: _wifiIcon
        color: Style.foreground
    }
}
