import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DualFuelHeatingPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property bool isCelsius: deviceController.temperatureUnit === AppSpec.Cel

    /* Object properties
     * ****************************************************************************************/
    title: "Dual Fuel Heating"
    rightPadding: 20 * scaleFactor
    leftPadding: 20 * scaleFactor
    topPadding: scaleFactor


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
        anchors.rightMargin: 10
        anchors.bottomMargin: 10

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
        // anchors.centerIn: parent
        anchors.top: parent.top
        width: parent.width
        spacing: initialSetup ? -5 : 4

        //! Emergency Heating
        RowLayout {
            spacing: 24
            Label {
                Layout.fillWidth: true
                text: "Emergency Heating"
            }

            Switch {
                id: emergencyHeatingSwh

                checked: appModel.systemSetup.heatPumpEmergency
            }
        }


        //! Emergency Heat Stages
        RowLayout {
            spacing: 24

            visible: false
            Label {
                Layout.fillWidth: true
                text: "Em. Heat Stages"
            }

            RowLayout {
                id: emHeatStageLayout

                Layout.fillWidth: false

                property int emHeatStage: 1


                RadioButton {
                    // checked: appModel.systemSetup.heatStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            emHeatStageLayout.emHeatStage = Number(text);
                    }

                    text: "1"
                }

                RadioButton {
                    // checked: appModel.systemSetup.heatStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            emHeatStageLayout.emHeatStage = Number(text);
                    }

                    text: "2"
                }
            }
        }

        //! Heat Pump Stages
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
                    checked: appModel.systemSetup.coolStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            heatPumpStageLayout.heatPumpStage = Number(text);
                    }

                    text: "1"
                }

                RadioButton {
                    checked: appModel.systemSetup.coolStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            heatPumpStageLayout.heatPumpStage = Number(text);
                    }

                    text: "2"
                }
            }
        }

        //! O/B on State
        RowLayout {
            id: heatPumpOBStateLayout

            spacing: 24

            property int heatPumpOBState: 1

            Label {
                text: "O/B on State"
            }

            Item {

                Layout.fillWidth: true
                height: parent.height
            }

            RowLayout {
                Layout.fillWidth: true

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


        //! Furnace Stages
        RowLayout {
            spacing: 24

            Label {
                Layout.fillWidth: true
                text: "Furnace Stages"
            }

            RowLayout {
                id: furnaceStageLayout

                Layout.fillWidth: false

                property int furnaceStage: 1


                RadioButton {
                    checked: appModel.systemSetup.heatStage !== 2
                    onCheckedChanged: {
                        if (checked)
                            furnaceStageLayout.furnaceStage = Number(text);
                    }

                    text: "1"
                }

                RadioButton {
                    checked: appModel.systemSetup.heatStage === Number(text)
                    onCheckedChanged: {
                        if (checked)
                            furnaceStageLayout.furnaceStage = Number(text);
                    }

                    text: "2"
                }
            }
        }

        ColumnLayout {

            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text: "Use furnace to heat when temperature outside is below"
                wrapMode: Text.WordWrap
                font.pointSize: Qt.application.font.pointSize * 0.9
            }

            SingleIconSlider {
                id: temperature

                Layout.fillWidth: true

                icon: "\uf2c8"
                labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(deviceController.temperatureUnit))
                from: isCelsius ? Utils.fahrenheitToCelsius(AppSpec.minimumDualFuelThresholdF) : AppSpec.minimumDualFuelThresholdF;
                to:   isCelsius ? Utils.fahrenheitToCelsius(AppSpec.maximumDualFuelThresholdF) : AppSpec.maximumDualFuelThresholdF;

                control.value: Utils.convertedTemperature(appModel.systemSetup.dualFuelThreshod, deviceController.temperatureUnit);
            }
        }
    }

    /* Functions
     * ****************************************************************************************/

    function updateModel() {
        if (deviceController) {
            var temperatureC = isCelsius ? temperature.control.value : Utils.fahrenheitToCelsius(temperature.control.value);
            deviceController.setSystemDualFuelHeating(emergencyHeatingSwh.checked,
                                                      heatPumpStageLayout.heatPumpStage,
                                                      furnaceStageLayout.furnaceStage,
                                                      heatPumpOBStateLayout.heatPumpOBState,
                                                      temperatureC)
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
