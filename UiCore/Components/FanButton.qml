import QtQuick
import QtQuick.Layouts

import Ronia
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
    padding: AppStyle.size / 90
    flat: true
    checkable: true
    checked: false //! \todo: connect to a property in Fan

    /* Childrent
     * ****************************************************************************************/
    Image {
        anchors.centerIn: parent
        width: _root.icon.width *  AppStyle.size / 480
        height: _root.icon.height * AppStyle.size / 480
        source: _root.checked ? "qrc:/Stherm/Images/fan-on.png" : "qrc:/Stherm/Images/fan-off.png"
    }
}
