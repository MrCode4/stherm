import QtQuick
import Qt5Compat.GraphicalEffects

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiIcon visualize a wifi strength inside a WifiDelegate
 * ***********************************************************************************************/
Item {
    id: _root
    /* Property declaration
     * ****************************************************************************************/
    //! this is for wifi delagates to show as connected on top
    //! and on home page will be used for no Wi-Fi
    property bool       isConnected: true

    //! to show indicator on connected Wi-Fi
    property bool       hasInternet: false

    //! true on home page
    property bool       showOff: false

    //! Strength of wifi (0, 100)
    property int        strength

    /* Children
     * ****************************************************************************************/
    Image {
        id: _wifiImage
        anchors.fill: parent

        fillMode: Image.PreserveAspectFit
        smooth: true
        source: isConnected || !showOff
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


    Label {
        anchors.centerIn: parent
        visible: isConnected && !NetworkInterface.hasInternet
        // to distinguish with wifi icon
        leftPadding: 1;
        rightPadding: 1;
        font {
            weight: Font.Bold
            pointSize: Application.font.pointSize * 1.2
        }
        text: "!"
        horizontalAlignment: Text.AlignHCenter
        // make it more aesthetic
        background: Rectangle {
            color: Style.background
            radius: 2
        }
    }
}
