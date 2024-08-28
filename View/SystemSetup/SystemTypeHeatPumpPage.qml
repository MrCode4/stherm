import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeHeatPumpPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 48
    rightPadding: 48
    title: "Heat Pump"

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
            updateModel();

            //! Also move out of this Page
            goToSystemTypePage();
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
                    checked: appModel.systemSetup.heatStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            heatPumpStageLayout.heatPumpStage = Number(text);
                    }

                    text: "1"
                }

                RadioButton {
                    checked: appModel.systemSetup.heatStage === Number(text)
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

    function updateModel() {
        if (deviceController) {
            deviceController.setSystemHeatPump(_emergencyHeatingSwh.checked,
                                               heatPumpStageLayout.heatPumpStage,
                                               heatPumpOBStateLayout.heatPumpOBState)
        }
    }

    function goToSystemTypePage()
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
