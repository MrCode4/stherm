import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InternalSensorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property Declaration
     * ****************************************************************************************/
    property var model: ({})


    property real rangeMilliMeterMin: 60
    property real rangeMilliMeterMax: 70
    property real tvocMin: 0
    property real tvocMax: 100
    property real brighnessMin: 0
    property real brighnessMax: 100
    property real co2Min: 0
    property real co2Max: 1
    property real etohMin: 0
    property real etohMax: 100
    property real fanSpeedMin: 3500
    property real fanSpeedMax: 6000
    property real humidityMin: 0
    property real humidityMax: 100
    property real iaqMin: 0
    property real iaqMax: 100
    property real pressureMin: 0
    property real pressureMax: 100
    property real temperatureMin: -20
    property real temperatureMax: 50

    //! autoNext to disable next timer when use back button in the next page
    property bool autoNext: true

    /* Object properties
     * ****************************************************************************************/
    title: "Internal Sensor Test"

    Component.onCompleted: {
        model = deviceController.getTestData();
    }

    //! Repeat the test when the page is visible.
    onVisibleChanged: {
        if (visible) {
            if (!overrideBtn.checked)
                updatingModelTimer.start();

        }  else {
            autoNext = false;
        }
    }

    /* Functions
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

    function nextPage(){
        writeSensorData()

        // show relays test always for now
        if (deviceController.startMode !== 0 || true) {
            //! Next page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/Test/RelayTestPage.qml", {
                                             "uiSession": uiSession,
                                             "backButtonVisible" : backButtonVisible
                                         })
            }
        } else {
            //! Test mode enabled with GPIO as there is no ti board connected
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/Test/QRCodeTestPage.qml", {
                                             "uiSession": uiSession,
                                             "backButtonVisible" : backButtonVisible
                                         })
            }
        }
    }

    /* Children
     * ****************************************************************************************/

    //! Next button
    ToolButton {
        id: nextBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            backButtonVisible = true;
            nextPage();
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
                temperatureField.forceActiveFocus();
            else
                deviceController.setTestData(model.temperature, true);
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
        id: updatingModelTimer

        interval: 1000
        repeat: true
        running: root.visible && !overrideBtn.checked
        onTriggered: {
            root.model = deviceController.getTestData();
            autoNext = true;
        }
    }

    //! timer for next page
    Timer {
        interval: 100
        repeat: false
        running: root.visible && root.autoNext && temperatureField.valid &&
                 humidityField.valid && tofField.valid && ambientField.valid &&
                 co2Field.valid && fanField.valid
        onTriggered: {
            backButtonVisible = false;
            nextPage()
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

            property bool valid : (root.model?.temperature >= temperatureMin && root.model?.temperature <= temperatureMax) ?? false

            readOnly: !overrideBtn.checked
            Layout.preferredHeight: 50
            text: readOnly ? (root.model?.temperature?.toFixed(3)  ?? "") : text
            color:  valid ? Material.foreground : Style.testFailColor

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
            id: humidityField

            property bool valid : (root.model?.humidity >= humidityMin && root.model?.humidity <= humidityMax) ?? false

            text: root.model?.humidity  ?? "NaN"
            Layout.preferredWidth:  temperatureField.width
            color: valid ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "TOF :"
        }

        Label {
            id: tofField

            property bool valid : (root.model?.RangeMilliMeter >= rangeMilliMeterMin && root.model?.RangeMilliMeter <= rangeMilliMeterMax) ?? false

            text: (root.model?.RangeMilliMeter > 0 && root.model?.RangeMilliMeter < 1000) ? ("ON (" + root.model.RangeMilliMeter + ")") : "OFF"
            Layout.preferredWidth:  temperatureField.width
            color: valid ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "Ambient :"
        }

        Label {
            id: ambientField

            property bool valid : (root.model?.brighness >= brighnessMin && root.model?.brighness <= brighnessMax) ?? false

            text:root.model?.brighness  ?? ""
            Layout.preferredWidth:  temperatureField.width
            color: valid ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "CO2 :"
        }

        Label {
            id: co2Field

            property bool valid : (root.model?.iaq >= iaqMin && root.model?.iaq <= iaqMax) ?? false

            text:root.model?.iaq  ?? ""
            Layout.preferredWidth:  temperatureField.width
            color: valid ? Material.foreground : Style.testFailColor
        }

        Label {
            text: "Fan Speed:"
        }

        Label {
            id: fanField

            property bool valid : (root.model?.fanSpeed >= fanSpeedMin && root.model?.fanSpeed <= fanSpeedMax) ?? false

            text: root.model?.fanSpeed  ?? ""
            Layout.preferredWidth:  temperatureField.width
            color: valid ? Material.foreground : Style.testFailColor
        }
    }
}
