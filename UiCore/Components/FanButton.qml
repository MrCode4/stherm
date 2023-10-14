import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * FanButton is a button for controlling fan
 * ***********************************************************************************************/
ToolButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to Fan
    property Fan    fan

    /* Object properties
     * ****************************************************************************************/
    flat: true
    checkable: true
    checked: false //! \todo: connect to a property in Fan

    /* Childrent
     * ****************************************************************************************/
    Image {
        anchors.centerIn: parent
        width: _root.icon.width
        height: _root.icon.height
        source: _root.checked ? "qrc:/Stherm/Images/fan-on.png" : "qrc:/Stherm/Images/fan-off.png"
    }
}
