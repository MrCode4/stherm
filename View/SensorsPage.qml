import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

import "."
/*! ***********************************************************************************************
 * SensorsPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    title: "Sensors"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 48
        }
        width: parent.width * 0.65
        height: Math.min(implicitHeight, _root.availableHeight)

        Button {
            Layout.alignment: Qt.AlignCenter
            text: "Onboard Sensor"
            checkable: true
            checked: true
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 32
            opacity: 0.8
            text: "Wireless Sensors"
        }

        //! Sensors ListView
        ListView {
            id: _sensorsLv

            ScrollIndicator.vertical: ScrollIndicator { }
            Layout.fillHeight: true
            implicitHeight: contentHeight
            model: appModel?.sensors ?? []
            spacing: 12
            delegate: SensorDelegate {
                required property var modelData
                required property int index

                width: ListView.view.width
                height: implicitHeight
                sensor: modelData instanceof Sensor ? modelData : null
                delegateIndex: index
            }
        }
    }
}
