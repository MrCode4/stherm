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
        source: {
            if (deviceController.activeSystemMode === AppSpec.Cooling) {
                return "qrc:/Stherm/Images/fan-cool.svg";

            } else if (deviceController.activeSystemMode === AppSpec.Heating ||
                       deviceController.activeSystemMode === AppSpec.Emergency) {
                return "qrc:/Stherm/Images/fan-heat.svg";
            }

            return "qrc:/Stherm/Images/fan-on.png";
        }

        //! Animation for rotatin
        //! will cost almost 10% cpu usage even when not visible
        Behavior on rotation {
            enabled: false // fanAnimation.running
            NumberAnimation
            {
                duration: fanAnimation.interval
            }
        }
    }

    //! Timer for fan animation
    //! we may lower the fps in quiet mode
    Timer {
        id: fanAnimation

        interval: 60
        running: false
        repeat: true

        onTriggered: {
            logoImage.rotation += 10;
        }
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

        //! Animate the fan image
        function onFanWorkChanged(fanState: bool) {
            if (fanState) {
                fanAnimation.start();

            } else {
                fanAnimation.stop();
                logoImage.rotation = 0;
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
