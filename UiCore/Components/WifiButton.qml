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
    WifiIcon {
        id: _wifiIcon
        anchors.centerIn: parent
        width: _root.width - 8
        height: width

        isConnected: Boolean(NetworkInterface.connectedWifi)
        strength: NetworkInterface.connectedWifi?.strength ?? 0
    }
}
