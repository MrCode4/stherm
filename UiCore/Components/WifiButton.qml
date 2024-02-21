import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiButton provide a ui for connecting to a wifi network and show status of current connection
 * ***********************************************************************************************/
ToolButton {
    id: root

    /* Property declration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    touchMargin: 10

    /* Children
     * ****************************************************************************************/

    Image {
        id: _wifiImage

        property bool isConnected: Boolean(NetworkInterface.connectedWifi)
        property bool hasInternet: NetworkInterface.hasInternet
        property int  strength: NetworkInterface.connectedWifi?.strength ?? 0

        anchors.fill: parent
        anchors.margins: root.padding

        fillMode: Image.PreserveAspectFit
        smooth: true
        source: isConnected
                ? (hasInternet ? (strength > 79
                                  ? "qrc:/Stherm/Images/Wifi/wifi.png"
                                  : (strength > 50
                                     ? "qrc:/Stherm/Images/Wifi/wifi-good.png"
                                     : (strength > 25
                                        ? "qrc:/Stherm/Images/Wifi/wifi-fair.png"
                                        : "qrc:/Stherm/Images/Wifi/wifi-weak.png"
                                        )
                                     )
                                  )
                               : "qrc:/Stherm/Images/Wifi/wifi-no-internet.png")
                : "qrc:/Stherm/Images/Wifi/wifi-off.png"
        sourceSize.width: width
        sourceSize.height: height
    }
}
