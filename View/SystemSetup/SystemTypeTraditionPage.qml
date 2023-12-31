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
    backButtonCallback: function() {
        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                //! Pop twice to get back to SystemSetupPage
                _root.StackView.view.pop();
                _root.StackView.view.pop();
            }
        }
    }

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
            if (deviceController) {
                deviceController.setSystemTraditional(traditionalCoolStageLayout.traditionalCoolStage,
                                                      traditionalHeatStageLayout.traditionalHeatStage)
            }

            //! Also move out of this Page
            backButtonCallback();
        }
    }

    GridLayout {
        anchors.centerIn: parent
        columns: 4
        columnSpacing: 20
        rowSpacing: 16

        Label {
            text: "Cool Stages"
        }

        RowLayout {
            id: traditionalCoolStageLayout

            Layout.columnSpan: 3

            property int traditionalCoolStage: traditionalCoolStage_1_RB.checked ? Number(traditionalCoolStage_1_RB.text) :
                                                                                   Number(traditionalCoolStage_2_RB.text);

            RadioButton {
                id: traditionalCoolStage_1_RB

                checked: appModel.systemSetup.coolStage === Number(text)
                text: "1"
            }

            RadioButton {
                id: traditionalCoolStage_2_RB

                checked: appModel.systemSetup.coolStage === Number(text)
                text: "2"
            }
        }

        Label {
            text: "Heat Stages"
        }

        RowLayout {
            id: traditionalHeatStageLayout

            Layout.columnSpan: 3

            property int traditionalHeatStage: 1

            RadioButton {
                checked: appModel.systemSetup.heatStage === Number(text)
                onCheckedChanged:  {
                    if (checked)
                        traditionalHeatStageLayout.traditionalHeatStage = Number(text);
                }

                text: "1"
            }

            RadioButton {
                checked: appModel.systemSetup.heatStage === Number(text)
                onCheckedChanged: {
                    if (checked)
                        traditionalHeatStageLayout.traditionalHeatStage = Number(text);
                }

                text: "2"
            }

            RadioButton {
                checked: appModel.systemSetup.heatStage === Number(text)
                onCheckedChanged: {
                    if (checked)
                        traditionalHeatStageLayout.traditionalHeatStage = Number(text);
                }

                text: "3"
            }
        }
    }
}
