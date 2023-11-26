import QtQuick
import Qt5Compat.GraphicalEffects

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiIcon visualize a wifi strength
 * ***********************************************************************************************/
Item {
    id: _root
    /* Property declaration
     * ****************************************************************************************/
    //!
    property bool       isConnected: true

    //! Strength of wifi (0, 100)
    property int        strength

    /* Children
     * ****************************************************************************************/
    Image {
        id: _wifiImage
        anchors.fill: parent

        fillMode: Image.PreserveAspectFit
        smooth: true
        source: isConnected
                ? (strength > 79
                   ? "qrc:/Stherm/Images/Wifi/wifi.png"
                   : (strength > 50
                      ? "qrc:/Stherm/Images/Wifi/wifi-good.png"
                      : (strength > 25
                         ? "qrc:/Stherm/Images/Wifi/wifi-fair.png"
                         : "qrc:/Stherm/Images/Wifi/wifi-weak.png"
                         )
                      )
                   )
                : "qrc:/Stherm/Images/Wifi/wifi-off.png"
        sourceSize.width: width
        sourceSize.height: height
    }
}
