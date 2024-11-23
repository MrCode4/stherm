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
            text: FAIcons.check
        }

        visible: !initialSetup

        onClicked: {
            //! Do neccessary updates
            updateModel();

            //! Also move out of this Page
            goToSystemTypePage();
        }
    }

    Flickable {
        id: contentFlickable

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: _contentCol.implicitHeight

        ColumnLayout {
            id: _contentCol

            width: parent.width
            height:  Math.max(_contentCol.implicitHeight, root.height * 0.85)
            spacing: 8
            // spacing: initialSetup ? -5 : 4


            //! Heat Pump Stages
            RowLayout {
                spacing: 12

                Label {
                    Layout.fillWidth: true
                    text: "Heat Pump Stages"
                }

                RowLayout {
                    id: heatPumpStageLayout

                    Layout.fillWidth: false

                    property int heatPumpStage: 1

                    spacing: 5

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

                spacing: 12

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


            //! Auxiliary Stages
            RowLayout {
                spacing: 12

                Label {
                    Layout.fillWidth: true
                    text: "Auxiliary Stages"
                }

                RowLayout {
                    id: auxStageLayout

                    Layout.fillWidth: false

                    property int auxStage: 1


                    RadioButton {
                        // Default is 1
                        checked: appModel.systemSetup.heatStage !== 2 && appModel.systemSetup.heatStage !== 3
                        onCheckedChanged: {
                            if (checked)
                                auxStageLayout.auxStage = Number(text);
                        }

                        text: "1"
                    }

                    RadioButton {
                        checked: appModel.systemSetup.heatStage === Number(text)
                        onCheckedChanged: {
                            if (checked)
                                auxStageLayout.auxStage = Number(text);
                        }

                        text: "2"
                    }

                    RadioButton {
                        checked: appModel.systemSetup.heatStage === Number(text)
                        onCheckedChanged: {
                            if (checked)
                                auxStageLayout.auxStage = Number(text);
                        }

                        text: "3"
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 10

                text: "Do you want the thermostat to automatically switch over to auxiliary heat?"
                font.pointSize: Application.font.pointSize * 0.9
                wrapMode: Text.WordWrap
            }

            //! Auxiliary Control Type
            RowLayout {
                Layout.fillWidth: true

                //! Manual emenrgency: When users select Manual for emergency heating, an additional button labeled Emergency appears in the System Mode menu
                RadioButton {
                    id: yesAutoRB

                    checked: appModel.systemSetup?.isAUXAuto ?? true
                    text: "Yes"
                }

                RadioButton {
                    id: noAutoRB

                    text: "No"
                    checked: !(appModel.systemSetup?.isAUXAuto ?? true)
                }
            }

            ColumnLayout {

                Layout.fillWidth: true

                visible: yesAutoRB.checked

                Label {
                    Layout.fillWidth: true
                    text: "Use auxiliary to heat when temperature outside is below"
                    wrapMode: Text.WordWrap
                    font.pointSize: Qt.application.font.pointSize * 0.9
                }

                SingleIconSlider {
                    id: temperature

                    Layout.fillWidth: true

                    icon: FAIcons.temperatureThreeQuarters
                    labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(deviceController.temperatureUnit))
                    from: isCelsius ? Utils.fahrenheitToCelsius(AppSpec.minimumDualFuelThresholdF) : AppSpec.minimumDualFuelThresholdF;
                    to:   isCelsius ? Utils.fahrenheitToCelsius(AppSpec.maximumDualFuelThresholdF) : AppSpec.maximumDualFuelThresholdF;

                    control.value: Utils.convertedTemperature(appModel.systemSetup.dualFuelThreshod, deviceController.temperatureUnit);
                }
            }

            CautionRectangle {
                Layout.topMargin: 15
                Layout.fillWidth: true

                visible: noAutoRB.checked
                icon: FAIcons.circleInfo
                iconColor: "#94A3B8"
                height: 90
                text: "The auxiliary (Aux) can be only used when manually selected from system mode."
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 10

                visible: noAutoRB.checked
                text: "Please select the heating type during Auto and Vacation modes"
                font.pointSize: Application.font.pointSize * 0.9
                wrapMode: Text.WordWrap
            }

            //! Auxiliary Control Type
            RowLayout {
                Layout.fillWidth: true

                visible: noAutoRB.checked

                RadioButton {
                    id: manualHeatPumpRadio

                    checked: appModel.systemSetup?.dualFuelHeatingModeDefault !== AppSpec.DFMAuxiliary ?? true
                    text: "Heat Pump"
                }

                RadioButton {
                    id: manualAuxRadio

                    text: "Auxiliary"
                    checked: appModel.systemSetup?.dualFuelHeatingModeDefault === AppSpec.DFMAuxiliary ?? false
                }
            }

            CautionRectangle {
                Layout.topMargin: 15
                Layout.fillWidth: true

                visible: noAutoRB.checked
                icon: FAIcons.circleInfo
                iconColor: "#94A3B8"
                height: 95
                text: "The selected heat type will stay as a default until manually changed from the system setup."
            }


            Item {
                Layout.fillWidth: true
                height: 20
            }

            //! Next button in initial setup flow
            ButtonInverted {
                text: "Next"

                Layout.alignment: Qt.AlignRight | Qt.AlignBottom
                Layout.rightMargin: 10
                Layout.bottomMargin: 10

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
        }
    }
    /* Functions
     * ****************************************************************************************/

    function updateModel() {
        if (deviceController) {
            var temperatureC = isCelsius ? temperature.control.value : Utils.fahrenheitToCelsius(temperature.control.value);
            deviceController.setSystemDualFuelHeating(heatPumpStageLayout.heatPumpStage,
                                                      auxStageLayout.auxStage,
                                                      heatPumpOBStateLayout.heatPumpOBState,
                                                      temperatureC,
                                                      yesAutoRB.checked,
                                                      manualAuxRadio.checked ? AppSpec.DFMAuxiliary : AppSpec.DFMHeatPump)
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
