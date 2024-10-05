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

    //! stretch vertically to cover more area
    property real yFactor: 1.15 * _wifiImage?.height / (Math.max(20, (noInternetIndicator?.height ?? 20))) ?? 1;

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

    // aesthetic External background for noInternetIndicator
    Rectangle {
        visible: noInternetIndicator.visible
        anchors.horizontalCenter: noInternetIndicator.horizontalCenter
        anchors.top: noInternetIndicator.top
        // to distinguish with wifi icon
        width: noInternetIndicator.width + 2
        // to fit better on wifi signals on both sizes
        height: noInternetIndicator.height / 2 + (yFactor > 1.3 ? 7 : -3)
        anchors.topMargin: yFactor > 1.3 ? 7 : 5
        color: Style.background
        radius: 2
    }

    Label {
        id: noInternetIndicator
        visible: isConnected && !hasInternet
        anchors.centerIn: parent
        // Adjust this value to match point of ! and wifi
        anchors.verticalCenterOffset: yFactor > 1.3 ? -7 : 3
        font {
            weight: Font.Bold
            pointSize: Application.font.pointSize * 1.2
        }
        text: "!"
        horizontalAlignment: Text.AlignHCenter

        transform: Scale {
            xScale: 1
            yScale: yFactor > 1.3 ? yFactor : 1.0 // Scale vertically in home
        }
    }
}
