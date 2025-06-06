import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeTraditionPage handle setting values in tranditional system type
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Traditional"

    /* Children
     * ****************************************************************************************/

    //! Confirm/Next button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        visible: !initialSetup

        onClicked: {
            //! Do neccessary updates
            updateModel()

            //! Also move out of this Page
            goToSystemSetupPage();
        }
    }

    //! Next button in initial setup flow
    ButtonInverted {
        text: "Next"

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        visible: initialSetup
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            updateModel();

            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemAccessoriesPage.qml", {
                                              "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                          });
            }
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

    function updateModel() {
        if (deviceController) {
            deviceController.setSystemTraditional(traditionalCoolStageLayout.traditionalCoolStage,
                                                  traditionalHeatStageLayout.traditionalHeatStage)
        }
    }

    function goToSystemSetupPage()
    {
        if (initialSetup) {
            backButtonCallback();
            return;
        }

        if (root.StackView.view) {
            //! Then Page is inside an StackView
            if (root.StackView.view.currentItem == root) {
                //! Pop twice to get back to SystemSetupPage
                if (root.StackView.view.get(root.StackView.view.depth - 2, StackView.DontLoad) instanceof SystemTypePage) {
                    root.StackView.view.pop();
                }
                root.StackView.view.pop();
            }
        }
    }
}
