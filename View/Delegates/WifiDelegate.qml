import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 *
 * ***********************************************************************************************/
ItemDelegate {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! WifiInfo
    property WifiInfo   wifi

    //! Index in ListView
    property int        delegateIndex

    /* Children
     * ****************************************************************************************/
    RowLayout {
        x: 8
        width: parent.width - 16
        height: _root.Material.delegateHeight
        spacing: 12

        RoniaTextIcon {
            color: wifi?.connected ? _root.Material.accentColor : _root.Material.foreground
            text: "\uf1eb" //! wifi icon
        }

        Label {
            Layout.fillWidth: true
            color: wifi?.connected ? _root.Material.accentColor : _root.Material.foreground
            text: wifi?.ssid ?? ""
        }
    }

    Behavior on height { NumberAnimation { } }
}

