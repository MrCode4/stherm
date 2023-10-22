import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * WifiPage provides a ui to connect to a Wifi network
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Wifi Settings"

    /* Children
     * ****************************************************************************************/
    ListView {
        anchors.fill: parent
        model: NetworkInterface.wifis
        delegate: ItemDelegate {
            text: modelData.ssid
        }
    }

    Component.onCompleted: {
        NetworkInterface.refereshWifis();
    }
}
