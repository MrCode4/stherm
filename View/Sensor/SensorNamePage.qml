import QtQuick

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
    required property Sensor sensor

    /* object properties
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    TextField {
        id: sensorNameTf

        width: 200
        height: 80
        anchors.horizontalCenter: parent.horizontalCenter
        y: 60
    }
}
