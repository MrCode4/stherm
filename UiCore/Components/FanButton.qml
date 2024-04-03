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
    property DeviceController deviceController

    property I_Device appModel

    //! Reference to Fan
    property Fan    fan: appModel.fan

    /* Object properties
     * ****************************************************************************************/
    checkable: false
    checked: false //! \todo: connect to a property in Fan
    contentItem: Image {
        id: logoImage
        sourceSize.width: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
        sourceSize.height: Style.fontIconSize.largePt * 1.3334 //! 16px = 12pt
        source: "qrc:/Stherm/Images/fan-on.png"
    }

    Connections {
        target: deviceController.deviceControllerCPP

        function onCurrentSystemModeChanged(obState: int) {
            if (obState === AppSpec.Cooling) {
                logoImage.source = "qrc:/Stherm/Images/fan-cool.svg";

            } else if (obState === AppSpec.Heating) {
                logoImage.source = "qrc:/Stherm/Images/fan-cool.svg";

            } else {
                logoImage.source = "qrc:/Stherm/Images/fan-on.png";

            }
        }
    }

}
