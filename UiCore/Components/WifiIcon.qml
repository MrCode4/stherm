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
    //!
    property bool       isConnected: true

    //! Is wifi open (no password)
    property bool       isOpen: false

    //! Strength of wifi (0, 100)
    property int        strength

    /* Children
     * ****************************************************************************************/
    Image {
        id: _wifiImage
        anchors.fill: parent

        fillMode: Image.PreserveAspectFit
        smooth: true
        source: (strength > 79
                 ? "qrc:/Stherm/Images/Wifi/wifi.png"
                 : (strength > 50
                    ? "qrc:/Stherm/Images/Wifi/wifi-good.png"
                    : (strength > 25
                       ? "qrc:/Stherm/Images/Wifi/wifi-fair.png"
                       : "qrc:/Stherm/Images/Wifi/wifi-weak.png"
                       )
                    )
                 )
        sourceSize.width: width
        sourceSize.height: height
    }

    RoniaTextIcon {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        //! strength > 0 means don't display lock for non-in-range wifis
        visible: strength > 0 && !isOpen
        font.pointSize: Application.font.pointSize * 0.7
        text: FAIcons.lock
    }

    Label {
        anchors.centerIn: parent
        visible: isConnected && !NetworkInterface.hasInternet
        height: contentHeight
        leftPadding: 1; rightPadding: 1
        font {
            weight: Font.Bold
            pointSize: Application.font.pointSize * 1.8
        }
        text: "!"
        horizontalAlignment: Text.AlignHCenter
        background: Rectangle {
            color: Style.background
            radius: 2
        }
    }
}
