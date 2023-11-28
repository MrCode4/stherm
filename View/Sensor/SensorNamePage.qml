import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorNamePage displays a text field to set name of a Sensor
 * ***********************************************************************************************/
Control {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Instance to the Sensor
    required property   Sensor sensor

    //! Selected name
    property alias      sensorName:     sensorNameTf.text

    /* object properties
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent.width * 0.6
        anchors.horizontalCenter: parent.horizontalCenter
        y: 40

        Label {
            Layout.alignment: Qt.AlignCenter
            font.bold: true
            text: "New Name"
        }

        TextField {
            id: sensorNameTf

            Layout.fillWidth: true
            y: 60
        }
    }
}
