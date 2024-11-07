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
    property bool initialSetup: true

    property bool isFahrenheit: deviceController.temperatureUnit != AppSpec.TempratureUnit.Cel

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 10
    rightPadding: 10
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

    Flickable {
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

            RowLayout {
                spacing: 24

                Label {
                    text: "Heat Pump Stages"
                }

                RowLayout {
                    id: heatPumpStageLayout

                    Layout.fillWidth: true

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

            RowLayout {
                id: heatPumpOBStateLayout

                spacing: 24

                property int heatPumpOBState: 1

                Label {
                    text: "O/B on State"
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

            RowLayout {
                spacing: 24
                Label {
                    Layout.fillWidth: true
                    text: "Auxiliary Heating"
                }

                Switch {
                    id: auxiliaryHeatingSwh
                    checked: appModel.systemSetup.auxiliaryHeating
                }
            }

            ColumnLayout {
                width: parent.width
                spacing: 8

                visible: auxiliaryHeatingSwh.checked

                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    text: "Set the minimum time for Auxiliary heat to run during the call."
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                SingleIconSlider {
                    id: minimumAuxiliaryTimeSlider

                    Layout.fillWidth: true
                    Layout.leftMargin: 5
                    Layout.rightMargin: 5
                    Layout.topMargin: 10

                    height: 90
                    leftSideColor: "#9BD2F7"
                    rightSideColor: "#484848"
                    icon: FAIcons.clockThree
                    iconSize: Qt.application.font.pointSize * 1.2
                    showRange: false
                    showTicks: true
                    title: "min"
                    control.stepSize: 1
                    ticksCount: 18
                    majorTickCount: 3
                    from: 1
                    to: 18
                    control.value: appModel.systemSetup.minimumAuxiliaryTime
                }

                CautionRectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    height: 70
                    text: "Settings wrong runtime can cause system damage."
                }

                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    text: "Would you like to control auxiliary heat manually or automatically (via thermostat)?"
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                //! Auxiliary Control Type
                RowLayout {
                    Layout.fillWidth: true

                    RadioButton {
                        id: manuallyRB

                        checked: appModel.systemSetup?.auxiliaryControlType !== AppSpecCPP.ACTAuto
                        text: "Manually"
                    }

                    RadioButton {
                        id: autoRB

                        text: "Auto"
                        checked: appModel.systemSetup?.auxiliaryControlType === AppSpecCPP.ACTAuto
                    }
                }

                CautionRectangle {
                    Layout.fillWidth: true
                    height: 100

                    visible: manuallyRB.checked
                    text: "Auxiliary heat will only activate when <span style='color:#ea0600;'>Auxiliary Heat(AUX)</span> is selected in system mode or when initiated by Defrost controller board (if equipped)."
                    icon: FAIcons.circleInfo
                    iconColor: "#94A3B8"
                }

                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    visible: autoRB.checked

                    text: "Set the temperature difference between the set point and room temperature to engage auxiliary heat."
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                SingleIconSlider {
                    id: temperatureDiffSlider

                    Layout.topMargin: 10
                    Layout.leftMargin: 5
                    Layout.rightMargin: 5

                    visible: autoRB.checked
                    leftSideColor: "#9BD2F7"
                    rightSideColor: "#484848"
                    icon: FAIcons.temperatureHigh
                    iconSize: Qt.application.font.pointSize * 1.2
                    showRange: false
                    showTicks: true
                    title: "Temp"
                    ticksCount: isFahrenheit? 18 : 10
                    majorTickCount: isFahrenheit ? 3 : 2
                    from: isFahrenheit ? 2 : 1.1
                    to:   isFahrenheit ? 3.8 : 2.1
                    labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(deviceController.temperatureUnit))
                    scaleValue: 10
                    control.stepSize: 0.1
                    control.value:  appModel.systemSetup.auxiliaryTemperatureDiffrence * (deviceController.temperatureUnit === AppSpec.TempratureUnit.Fah ? 1.8 : 1) * scaleValue
                }

                CautionRectangle {
                    Layout.topMargin: 15
                    Layout.fillWidth: true

                    visible: autoRB.checked && !temperatureDiffSlider.control.pressed &&
                             (temperatureDiffSlider.value  !== (isFahrenheit ? AppSpec.defaultAuxiliaryTemperatureDiffrenceC :
                                                                               AppSpec.defaultAuxiliaryTemperatureDiffrenceF))
                    height: 90
                    text: `Using the auxiliary heating is expensive. Recommended value is ${deviceController.temperatureUnit == AppSpec.TempratureUnit.Fah ? 2.9 : 1.6}\u00b0${AppSpec.temperatureUnitString(deviceController.temperatureUnit)}.`
                }

                Item {
                    Layout.fillWidth: true
                    height: 20
                }

            }

            Item {
                id: spacer
                Layout.fillWidth: true
                Layout.fillHeight: true
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

    function updateModel() {
        if (deviceController) {
            deviceController.setSystemHeatPump(auxiliaryHeatingSwh.checked,
                                               heatPumpStageLayout.heatPumpStage,
                                               heatPumpOBStateLayout.heatPumpOBState,
                                               minimumAuxiliaryTimeSlider.value,
                                               autoRB.checked ? AppSpecCPP.ACTAuto : AppSpecCPP.ACTManually,
                                               temperatureDiffSlider.value / (isFahrenheit ? 1.8 : 1))
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
