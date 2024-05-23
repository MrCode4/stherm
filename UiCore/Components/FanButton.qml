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

    //! Used to blink fan
    property real systemDelayCounter: -1

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


    //! blink during countdown
    //! CHECK, when fan is ON
    Timer {
        interval: 500
        running: systemDelayCounter >= 0
        repeat: true
        onTriggered: {
            systemDelayCounter -= 500;
            logoImage.visible = !logoImage.visible
        }
    }

    Connections {
        target: deviceController.deviceControllerCPP

        function onCurrentSystemModeChanged(state: int) {
            if (state === AppSpec.Cooling) {
                logoImage.source = "qrc:/Stherm/Images/fan-cool.svg";

            } else if (state === AppSpec.Heating || state === AppSpec.Emergency) {
                logoImage.source = "qrc:/Stherm/Images/fan-heat.svg";

            } else {
                logoImage.source = "qrc:/Stherm/Images/fan-on.png";
            }
        }

        function onStartSystemDelayCountdown(mode: int, delay: int) {
            systemDelayCounter = delay;
        }

        function onStopSystemDelayCountdown() {
            systemDelayCounter = -1;
            logoImage.visible = true;
        }
    }


}
