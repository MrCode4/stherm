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
    touchMargin: 30

    /* Children
     * ****************************************************************************************/

    WifiIcon {
        isConnected: Boolean(NetworkInterface.connectedWifi)
        hasInternet: NetworkInterface.hasInternet
        strength:    NetworkInterface.connectedWifi?.strength ?? 0
        showOff:     true

        anchors.fill: parent
        anchors.margins: root.padding + touchMargin / 4
    }
}
