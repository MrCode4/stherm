import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeTraditionPage handle setting values in tranditional system type
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Traditional"

    /* Children
     * ****************************************************************************************/
    GridLayout {
        anchors.centerIn: parent
        columns: 4
        columnSpacing: 20
        rowSpacing: 16

        Label {
            text: "Cool Stages"
        }

        RowLayout {
            Layout.columnSpan: 3

            RadioButton {
                checked: true
                text: "1"
            }

            RadioButton {
                text: "2"
            }
        }

        Label {
            text: "Heat Stages"
        }

        RowLayout {
            Layout.columnSpan: 3

            RadioButton {
                checked: true
                text: "1"
            }

            RadioButton {
                text: "2"
            }

            RadioButton {
                text: "3"
            }
        }
    }
}
