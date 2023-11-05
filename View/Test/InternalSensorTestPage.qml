import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InternalSensorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    title: "Internal Sensor Test"

    /* Children
     * ****************************************************************************************/
    GridLayout {
        anchors.fill: parent
        columns: 2

        Label {
            text: "Temprature :"
        }

        Item { }

        Label {
            text: "Humidity :"
        }

        Item { }

        Label {
            text: "TOF :"
        }

        Item { }

        Label {
            text: "Ambiend :"
        }

        Item { }

        Label {
            text: "CO2 :"
        }

        Item { }
    }
}
