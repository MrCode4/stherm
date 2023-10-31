import QtQuick
import QtQuick.Layouts

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
    padding: 8
    contentItem: RoniaTextIcon {
        font.pointSize: Style.fontIconSize.largePt - 1
        color: _root.Material.foreground
        text: {
            return NetworkInterface.connectedWifi
                    ? (NetworkInterface.connectedWifi.strength > 80
                       ? "\uf1eb" //! wifi icon
                       : (NetworkInterface.connectedWifi.strength > 50
                          ? "\uf6ab": //! wifi-fair icon
                            (NetworkInterface.connectedWifi.strength > 25 ? "\uf6aa" //! wifi-weak icon
                                                                          : "")
                                )
                             )
                    : "\uf6ac";
        }
    }

    Component.onCompleted: background.square = true
}
