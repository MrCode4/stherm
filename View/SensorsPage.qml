import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorsPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Sensor controller
    property SensorController       sensorController: uiSession?.sensorController ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Sensors"

    /* Children
     * ****************************************************************************************/
    //! Sensors ListView
    ListView {
        id: _sensorsLv

        ScrollIndicator.vertical: ScrollIndicator { }

        anchors.centerIn: parent
        width: parent.width * 0.6
        height: Math.min(contentHeight, parent.height)
        model: sensorController?.sensors ?? 0
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
