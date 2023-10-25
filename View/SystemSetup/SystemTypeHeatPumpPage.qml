import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeHeatPumpPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 48
    rightPadding: 48
    title: "Heat Pump"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width
        spacing: 16

        RowLayout {
            spacing: 24
            Label {
                Layout.fillWidth: true
                text: "Emergency Heating"
            }

            Switch {
                id: _emergencyHeatingSwh
            }
        }

        RowLayout {
            spacing: 24

            Label {
                Layout.fillWidth: true
                text: "Heat Pump Stages"
            }

            RowLayout {
                Layout.fillWidth: false

                RadioButton {
                    checked: true
                    text: "1"
                }

                RadioButton {
                    text: "2"
                }
            }
        }

        RowLayout {
            spacing: 24

            Label {
                Layout.fillWidth: true
                text: "O/B on State"
            }

            RowLayout {
                Layout.fillWidth: false

                RadioButton {
                    checked: true
                    text: "Cool"
                }

                RadioButton {
                    text: "Heat"
                }
            }
        }
    }
}
