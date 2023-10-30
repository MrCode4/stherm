import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSensorPage provides the ability to add a new sensor
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Add Sensor"

    /* Children
     * ****************************************************************************************/
    StackView {
        id: _pageStack
        anchors.fill: parent

        initialItem: _sensorMessageItem
    }

    Item {
        id: _sensorMessageItem
        visible: false

        Label {
            anchors.centerIn: parent
            width: parent.width * 0.85
            padding: 32
            text: "Please remove battery tab from the sensor or push reset button for 2 "
                  + "seconds to start pairing."
            wrapMode: "WordWrap"

            background: Rectangle {
                color: "transparent"
                radius: 32
                border.width: 2
                border.color: _root.Material.foreground
            }
        }
    }
}
