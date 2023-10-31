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
    checkable: true
    checked: false //! \todo: connect to a property in Fan
    contentItem: Image {
        sourceSize.width: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
        sourceSize.height: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
        source: _root.checked ? "qrc:/Stherm/Images/fan-on.png" : "qrc:/Stherm/Images/fan-off.png"
    }
}
