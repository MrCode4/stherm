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

        Behavior on contentY {
            NumberAnimation {
                duration: 250
            }
        }

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

            //! Auxiliary Heating
            RowLayout {
                spacing: 24
                Label {
                    Layout.fillWidth: true
                    text: "Auxiliary Heating"
                }

                Switch {
                    id: auxiliaryHeatingSwh
                    checked: appModel.systemSetup.auxiliaryHeating ?? true
                }
            }

            ColumnLayout {
                width: parent.width
                spacing: 8

                visible: auxiliaryHeatingSwh.checked

                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    text: "Set the minimum time for Aux heat to run during the call."
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                SingleIconSlider {
                    id: auxiliaryMinimumTimeSlider

                    Layout.fillWidth: true
                    Layout.leftMargin: 5
                    Layout.rightMargin: 10
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
                    ticksCount: 3
                    majorTickCount: 1
                    from: 2
                    to: 5
                    snapMode: Slider.SnapAlways
                    control.value: appModel.systemSetup.emergencyMinimumTime
                }

                //! Auxiliary Stages
                RowLayout {
                    spacing: 24

                    Label {
                        text: "Auxiliary Stages"
                    }

                    RowLayout {
                        id: auxiliaryStageLayout

                        Layout.fillWidth: true

                        property int auxiliaryStages: appModel.systemSetup.heatStage


                        RadioButton {
                            checked: appModel.systemSetup.heatStage !== 2
                            onCheckedChanged: {
                                if (checked)
                                    auxiliaryStageLayout.auxiliaryStages = Number(text);
                            }

                            text: "1"
                        }

                        RadioButton {
                            checked: appModel.systemSetup.heatStage === Number(text)
                            onCheckedChanged: {
                                if (checked)
                                    auxiliaryStageLayout.auxiliaryStages = Number(text);
                            }

                            text: "2"
                        }
                    }
                }

                //! Would you like to turn on auxiliary heating in parallel with your heat pump when it's cold outside and the heat pump alone can't keep up?
                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    text: "Would you like to turn on auxiliary heating in parallel with your heat pump when it's cold outside and the heat pump alone can't keep up?"
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                //! Parallel auxiliary
                RowLayout {
                    Layout.fillWidth: true

                    RadioButton {
                        id: parallelAuxRB

                        checked: appModel.systemSetup?.useAuxiliaryParallelHeatPump ?? true
                        text: "Yes"
                    }

                    RadioButton {
                        id: autoRB

                        text: "No"
                        checked: !parallelAuxRB.checked
                    }
                }

                //! W1(Aux 1) and W3(E) terminals together
                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    text: "Do you want to drive W1(Aux 1) and W3(E) terminals together?"
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                RowLayout {
                    Layout.fillWidth: true

                    RadioButton {
                        id: w1w3AuxRB

                        checked: appModel.systemSetup?.driveAux1AndETogether ?? true
                        text: "Yes"
                    }

                    RadioButton {
                        text: "No"
                        checked: !w1w3AuxRB.checked
                    }
                }

                Label {
                    Layout.fillWidth: true
                    Layout.topMargin: 10

                    visible: auxiliaryStageLayout.auxiliaryStage === 2

                    text: "Do you want to drive all stages of auxiliary as Emergency?"
                    font.pointSize: Application.font.pointSize * 0.9
                    wrapMode: Text.WordWrap
                }

                //! Auxiliary as Emergency
                RowLayout {
                    Layout.fillWidth: true

                    visible: auxiliaryStageLayout.auxiliaryStage === 2

                    RadioButton {
                        id: auxiliaryAsEmergencyRB

                        checked: appModel.systemSetup?.enableEmergencyModeForAuxStages ?? true
                        text: "Yes"
                    }

                    RadioButton {
                        text: "No"
                        checked: !auxiliaryAsEmergencyRB.checked
                    }
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

    //! Timer to move the flickable to bottom
    //! Ensure to contentHeight is up-to-date.
    Timer {
        id: flickToBottomTimer

        interval: 50
        repeat: false
        running: false
        onTriggered: {
              contentFlickable.contentY  = contentFlickable.contentHeight - contentFlickable.height;
        }
    }

    function updateModel() {
        if (deviceController) {
            deviceController.setSystemHeatPump(auxiliaryHeatingSwh.checked,
                                               heatPumpStageLayout.heatPumpStage,
                                               heatPumpOBStateLayout.heatPumpOBState,
                                               auxiliaryMinimumTimeSlider.value,
                                               auxiliaryStageLayout.auxiliaryStages,
                                               parallelAuxRB.checked,
                                               w1w3AuxRB.checked,
                                               auxiliaryAsEmergencyRB.checked)
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
