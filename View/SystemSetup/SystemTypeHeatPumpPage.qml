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
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Do neccessary updates

            appModel.systemSetup.systemType = 1; // HeatPump: 1
            appModel.systemSetup.heatPumpEmergency = _emergencyHeatingSwh.checked;
            appModel.systemSetup.heatPumpStage     = heatPumpStageLayout.heatPumpStage;
            appModel.systemSetup.heatPumpOBState   = heatPumpOBStateLayout.heatPumpOBState;

            //! Also move out of this Page
            backButtonCallback();
        }
    }

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

                checked: appModel.systemSetup.heatPumpEmergency
            }
        }

        RowLayout {
            spacing: 24

            Label {
                Layout.fillWidth: true
                text: "Heat Pump Stages"
            }

            RowLayout {
                id: heatPumpStageLayout

                Layout.fillWidth: false

                property int heatPumpStage: 1


                RadioButton {
                    checked: appModel.systemSetup.heatPumpStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            heatPumpStageLayout.heatPumpStage = Number(text);
                    }

                    text: "1"
                }

                RadioButton {
                    checked: appModel.systemSetup.heatPumpStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            heatPumpStageLayout.heatPumpStage = Number(text);
                    }

                    text: "2"
                }
            }
        }

        RowLayout {
            id: heatPumpOBStateLayout

            spacing: 24

            property int heatPumpOBState: 1

            Label {
                Layout.fillWidth: true
                text: "O/B on State"
            }

            RowLayout {
                Layout.fillWidth: false

                RadioButton {
                    checked: appModel.systemSetup.heatPumpOBState === 0
                    onCheckedChanged: {
                        if (checked)
                            heatPumpOBStateLayout.heatPumpOBState = 0;
                    }
                    text: "Cool"
                }

                RadioButton {
                    checked: appModel.systemSetup.heatPumpOBState === 1
                    onCheckedChanged: {
                        if (checked)
                            heatPumpOBStateLayout.heatPumpOBState = 1;
                    }

                    text: "Heat"
                }
            }
        }
    }
}
