import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InternalSensorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    property var model: ({})

    title: "Internal Sensor Test"

    property real rangeMilliMeterMin: 0
    property real rangeMilliMeterMax: 1000
    property real tvocMin: 0
    property real tvocMax: 100
    property real brighnessMin: 0
    property real brighnessMax: 100
    property real co2Min: 0
    property real co2Max: 1000
    property real etohMin: 0
    property real etohMax: 100
    property real fanSpeedMin: 4000
    property real fanSpeedMax: 6000
    property real humidityMin: 0
    property real humidityMax: 100
    property real iaqMin: 0
    property real iaqMax: 100
    property real pressureMin: 0
    property real pressureMax: 100
    property real temperatureMin: -20
    property real temperatureMax: 50

    Component.onCompleted: root.model = deviceController.getTestData();

    /* Children
     * ****************************************************************************************/

    function writeSensorData() {
        let sensorData = deviceController.deviceControllerCPP.getMainData()

        for (let key in sensorData) {
            let value = sensorData[key]

            if (key === "RangeMilliMeter") {
                writeSensorResult(key, value, rangeMilliMeterMin, rangeMilliMeterMax)
            } else if (key === "Tvoc") {
                writeSensorResult(key, value, tvocMin, tvocMax)
            } else if (key === "brighness") {
                writeSensorResult(key, value, brighnessMin, brighnessMax)
            } else if (key === "co2") {
                writeSensorResult(key, value, co2Min, co2Max)
            } else if (key === "etoh") {
                writeSensorResult(key, value, etohMin, etohMax)
            } else if (key === "fanSpeed") {
                writeSensorResult(key, value, fanSpeedMin, fanSpeedMax)
            } else if (key === "humidity") {
                writeSensorResult(key, value, humidityMin, humidityMax)
            } else if (key === "iaq") {
                writeSensorResult(key, value, iaqMin, iaqMax)
            } else if (key === "pressure") {
                writeSensorResult(key, value, pressureMin, pressureMax)
            } else if (key === "temperature") {
                writeSensorResult(key, value, temperatureMin, temperatureMax)
            }
        }
    }

    function writeSensorResult(key, value, min, max) {
        let result = (value >= min && value <= max)
        let description = "%1 must be between %2 and %3. Value: %4".arg(key).arg(min).arg(max).arg(value)
        deviceController.deviceControllerCPP.writeTestResult(key, result, description)
    }

    //! Next button
    ToolButton {
        id: nextBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            writeSensorData()

            if (deviceController.startMode !== 0) {
                //! Next page
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/Test/RelayTestPage.qml", {
                                                 "uiSession": uiSession
                                             })
                }
            } else {
                //! Test mode enabled with GPIO as there is no ti board connected
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/Test/QRCodeTestPage.qml", {
                                                  "uiSession": uiSession
                                              })
                }
            }
        }
    }

    //! override button
    ToolButton {
        id: overrideBtn
        contentItem: RoniaTextIcon {
            text: FAIcons.penCircle
        }
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 5
        checkable: true
        checked: false
        onToggled: {
            if (checked)
                temperatureField.forceActiveFocus()
            else
                deviceController.setTestData(model.temperature, true)
        }
    }

    //! reset button
    ToolButton {
        id: resetBtn
        contentItem: RoniaTextIcon {
            text: FAIcons.revert
        }
        anchors.top: overrideBtn.bottom
        anchors.right: overrideBtn.right
        anchors.topMargin: 5
        visible: !overrideBtn.checked
        checkable: false
        onClicked: {
            deviceController.setTestData(model.temperature, false)
        }
    }

    //! timer for updating values
    Timer {
        interval: 1000
        repeat: true
        running: !overrideBtn.checked
        onTriggered: {
          root.model = deviceController.getTestData();
        }
    }

    //! items
    GridLayout {
        anchors.fill: parent
        columns: 2

        Label {
            text: "Temperature :"
        }

        TextField {
            id: temperatureField
            readOnly: !overrideBtn.checked
            Layout.preferredHeight: 50
            text: readOnly ? (root.model?.temperature.toFixed(3)  ?? "") : text
            color: (root.model?.temperature >= temperatureMin && root.model?.temperature <= temperatureMax) ? Material.foreground : Style.testFailColor

            validator: DoubleValidator {
                top: 40
                bottom: 0
                decimals: 1
            }

            onEditingFinished: {
                if (acceptableInput)
                    model.temperature = text
            }
        }

        Label {
            text: "Humidity :"
        }

        Label {
            text:root.model?.humidity  ?? "NaN"
            Layout.preferredWidth:  temperatureField.width
            color: (root.model?.humidity >= humidityMin && root.model?.humidity <= humidityMax) ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "TOF :"
        }

        Label {
            text: (root.model?.RangeMilliMeter > 0) ? ("ON (" + root.model.RangeMilliMeter + ")") : "OFF"
            Layout.preferredWidth:  temperatureField.width
            color: (root.model?.RangeMilliMeter >= rangeMilliMeterMin && root.model?.RangeMilliMeter <= rangeMilliMeterMax) ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "Ambient :"
        }

        Label {
            text:root.model?.brighness  ?? ""
            Layout.preferredWidth:  temperatureField.width
            color: (root.model?.brighness >= brighnessMin && root.model?.brighness <= brighnessMax) ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "CO2 :"
        }

        Label {
            text:root.model?.iaq  ?? ""
            Layout.preferredWidth:  temperatureField.width
            color: (root.model?.iaq >= iaqMin && root.model?.iaq <= iaqMax) ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "Fan Speed:"
        }

        Label {
            text:root.model?.fanSpeed  ?? ""
            Layout.preferredWidth:  temperatureField.width
            color: (root.model?.fanSpeed >= fanSpeedMin && root.model?.fanSpeed <= fanSpeedMax) ? Material.foreground : Style.testFailColor
        }
    }
}
