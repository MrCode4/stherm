import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DesiredTempratureItem provides a ui for setting desired temprature
 * ***********************************************************************************************/
Control {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    property  UiSession         uiSession

    property I_DeviceController deviceController: uiSession.deviceController

    property ScheduleCPP        currentSchedule: deviceController?.currentSchedule ?? null

    //! Reference to I_Device
    property I_Device           device: uiSession.appModel

    //! Temperature unit
    readonly property int       temperatureUnit:       device?.setting?.tempratureUnit ?? AppSpec.defaultTemperatureUnit

    //! Unit of temprature
    readonly property string    temperatureUnitString: AppSpec.temperatureUnitString(temperatureUnit)

    //! Minimum temprature
    property real               minTemprature: deviceController?._minimumTemperatureUI ?? 40

    //! Maximum temprature
    property real               maxTemprature: deviceController?._maximumTemperatureUI ?? 90

    //! Offset of desired temp label
    property int                labelVerticalOffset: -8

    //! Holds whether SemiCircleSlider is being dragged
    readonly property bool      dragging: _tempSlider.pressed || tempSliderDoubleHandle.pressed

    //! Whether labels moving anima, should be enabled
    property bool               enableAnimations: true

    //! Visibility of temprature label
    property bool               labelVisible: true

    //! Label width
    readonly property alias     labelWidth: rightTempLabel.width

    /* Object properties
     * ****************************************************************************************/
    onCurrentScheduleChanged: {
        if (currentSchedule) {
            Qt.callLater(autoModeTemperatureValueFromSchedule);

        } else if (device) {
            Qt.callLater(updateTemperatureValue, device.requestedTemp);
        }
    }

    onDraggingChanged: {
        deviceController.updateLockMode(AppSpec.EMDesiredTemperature, dragging);
        deviceController.updateLockMode(AppSpec.EMAutoMode, dragging);
    }

    font.pointSize: Qt.application.font.pointSize * 2.5
    background: null
    contentItem: Item {
        SemiCircleSlider {
            id: _tempSlider
            anchors.centerIn: parent
            width: parent.width
            height: width / 2
            enabled: labelVisible && !currentSchedule
            from: minTemprature
            to: maxTemprature

            //! Note: Can not bind the value as will be broken so we use Connections instead
            //! Also binding will be called before broken too many times as the values are loading one by one and
            //! can be faulty
            //! Use Component.onCompleted to update the UI for first time after the settings are loaded.
            Component.onCompleted: {
                var tmp = currentSchedule?.temprature ?? (device?.requestedTemp ?? AppSpec.defaultRequestedTemperature);
                _tempSlider.value = Utils.convertedTemperatureClamped(tmp,
                                                                      temperatureUnit,
                                                                      minTemprature,
                                                                      maxTemprature);
            }

            //! Use onPressed instead of on value changed so value is only applied to device when
            //! Dragging is finished
            onPressedChanged: {
                if (!pressed) {
                    updateTemperatureModel();
                }
            }
        }

        //! Double handle semi circle slider
        SemiCircleSliderDoubleHandle {
            id: tempSliderDoubleHandle

            anchors.centerIn: parent
            width: parent.width
            height: width / 2
            enabled: labelVisible && !currentSchedule
            difference: temperatureUnit === AppSpec.TempratureUnit.Fah ? AppSpec.autoModeDiffrenceF : AppSpec.autoModeDiffrenceC

            firstValueCeil: Utils.convertedTemperature(AppSpec.maxAutoMinTemp, temperatureUnit)
            secondValueFloor: Utils.convertedTemperature(AppSpec.minAutoMaxTemp, temperatureUnit)

            from: minTemprature
            to: maxTemprature

            onVisibleChanged: {
                if (visible) {
                    //! Set difference
                    tempSliderDoubleHandle.difference = temperatureUnit === AppSpec.TempratureUnit.Fah ? AppSpec.autoModeDiffrenceF : AppSpec.autoModeDiffrenceC
                    tempSliderDoubleHandle.updateFirstSecondValues();
                }
            }

            first.onPressedChanged: {
               updateAutoMinReqTempModel();
            }

            second.onPressedChanged: {
                updateAutoMaxReqTempModel();
            }

            //! Use Connections to update first and second values
            Connections {
                target: device ?? null
                enabled: tempSliderDoubleHandle.visible

                function onAutoMinReqTempChanged()
                {
                    tempSliderDoubleHandle.updateFirstValue();
                }

                function onAutoMaxReqTempChanged()
                {
                    tempSliderDoubleHandle.updateSecondValue();
                }
            }

            Connections {
                target: device?.setting ?? null
                enabled: tempSliderDoubleHandle.visible

                function onTempratureUnitChanged() {
                    if (currentSchedule)
                        autoModeTemperatureValueFromSchedule();
                    else
                        updateFirstSecondValuesTmr.restart();
                }
            }

            Connections {
                target: _root
                enabled: tempSliderDoubleHandle.visible && !currentSchedule

                function onMinTempratureChanged() { updateFirstSecondValuesTmr.restart(); }

                function onMaxTempratureChanged() { updateFirstSecondValuesTmr.restart(); }
            }

            Timer {
                id: updateFirstSecondValuesTmr
                interval: 50
                repeat: false
                running: false
                onTriggered: {
                    //! Set difference
                    tempSliderDoubleHandle.difference = temperatureUnit === AppSpec.TempratureUnit.Fah ? AppSpec.autoModeDiffrenceF : AppSpec.autoModeDiffrenceC
                    tempSliderDoubleHandle.updateFirstSecondValues();
                }
            }

            function updateAutoMinReqTempModel() {
                if (deviceController && !first.pressed) {
                    deviceController.setAutoMinReqTemp(temperatureUnit === AppSpec.TempratureUnit.Fah
                                                       ? Utils.fahrenheitToCelsius(first.value)
                                                       : first.value);
                    deviceController.updateEditMode(AppSpec.EMAutoMode);
                    deviceController.saveSettings();
                }
            }

            function updateAutoMaxReqTempModel() {
                if (deviceController && !second.pressed) {
                    deviceController.setAutoMaxReqTemp(temperatureUnit === AppSpec.TempratureUnit.Fah
                                                       ? Utils.fahrenheitToCelsius(second.value)
                                                       : second.value);
                    deviceController.updateEditMode(AppSpec.EMAutoMode);
                    deviceController.saveSettings();
                }
            }

            function updateFirstValue()
            {
                if (!device || currentSchedule) return;

                first.value = Utils.convertedTemperatureClamped(device.autoMinReqTemp,
                                                                temperatureUnit,
                                                                minTemprature,
                                                                maxTemprature);
            }

            function updateSecondValue()
            {
                if (!device || currentSchedule) return;

                second.value = Utils.convertedTemperatureClamped(device.autoMaxReqTemp,
                                                                 temperatureUnit,
                                                                 minTemprature,
                                                                 maxTemprature)
            }

            //! Update the first and second slider values.
            //! If the clamping logic has changed, review the corresponding functionality in the
            //! DeviceController class (specifically the setAutoTemperatureFromServer function).
            function updateFirstSecondValues()
            {
                if (!device || currentSchedule) return;

                var firstValue  = AppSpec.defaultAutoMinReqTemp;
                var secondValue = AppSpec.defaultAutoMaxReqTemp;
                var minimumSecondarySlider = Math.max(secondValueFloor, firstValue + tempSliderDoubleHandle.difference);

                // If both autoMinReqTemp and autoMaxReqTemp are zero, keep the default values.
                if (device.autoMinReqTemp !== 0 || device.autoMaxReqTemp !== 0) {
                    //! First calculate new values for handles without setting them
                    firstValue = Utils.convertedTemperatureClamped(device.autoMinReqTemp,
                                                                   temperatureUnit,
                                                                   minTemprature,
                                                                   maxTemprature - tempSliderDoubleHandle.difference);

                    // The minimum value for the second slider should be the greater of
                    // AppSpec.minAutoMaxTemp and the sum of the first slider value and the difference value.
                    // So we need recalculate it when first slider changed.
                    minimumSecondarySlider = Math.max(secondValueFloor, firstValue + tempSliderDoubleHandle.difference);
                    secondValue = Utils.convertedTemperatureClamped(device.autoMaxReqTemp,
                                                                    temperatureUnit,
                                                                    Math.max(minTemprature, minimumSecondarySlider),
                                                                    maxTemprature);
                }

                //! Now first set first.maxValue and second.minValue then update their actual values
                first.setMaxValue(Math.min(firstValueCeil, secondValue - tempSliderDoubleHandle.difference));
                second.setMinValue(minimumSecondarySlider);

                first.value = firstValue;
                updateAutoMinReqTempModel();

                second.value = secondValue;
                updateAutoMaxReqTempModel();

            }
        }

        //! Selected temprature item
        TempratureLabel {
            id: tempLbl

            anchors.right: parent.right
            anchors.rightMargin: 16 * scaleFactor
            anchors.top: parent.top

            showCurrentTemperature: false
            visible: dragging
            z: 1
            device: _root.uiSession.appModel

            // Show the pressed slider value
            temperature: {
                var temp = 0.0;

                if (_tempSlider.pressed) {
                    temp = _tempSlider.value.toFixed(0);

                } else if (tempSliderDoubleHandle.first.pressed) {
                    temp = tempSliderDoubleHandle.first.value.toFixed(0);

                } else if (tempSliderDoubleHandle.second.pressed) {
                    temp = tempSliderDoubleHandle.second.value.toFixed(0);
                }

                // The slider value is currently converted to the selected unit.
                // However, we need to convert it to Celsius.
                temp = temperatureUnit === AppSpec.TempratureUnit.Cel ? temp : Utils.fahrenheitToCelsius(temp);

                return temp;
            }
        }

        //! Min and Max temperature labels
        Label {
            y: _tempSlider.height
            x: 28 * scaleFactor
            opacity: 0.6
            font.pointSize: Application.font.pointSize * 0.9
            text: minTemprature
        }

        Label {
            y: _tempSlider.height
            x: parent.width - 28 * scaleFactor - width
            opacity: 0.6
            font.pointSize: Application.font.pointSize * 0.9
            text: maxTemprature
        }

        //! Cool-to and heat-to label
        Label {
            id: coolHeatLbl
            x: (parent.width - width) / 2
            font.pointSize: Application.font.pointSize * 0.8
            font.bold: true
        }

        //! Label to show desired temperature in cooling/heating mode and second temperature in auto
        Item {
            id: tempLabelParent
            width: _tempSlider.background.shapeWidth
            height: _tempSlider.background.shapeHeight
            anchors.centerIn: parent

            Label {
                id: rightTempLabel
                visible: labelVisible
                anchors {
                    verticalCenter: parent.verticalCenter
                    verticalCenterOffset: labelVerticalOffset
                }
                font {
                    pointSize: _root.font.pointSize * 0.65
                }

                //! Unit
                Row {
                    id: rightUnitLbl
                    anchors.left: parent.right
                    anchors.top: parent.top
                    opacity: 0.6

                    Label {
                        font.pointSize: Application.font.pointSize * 0.8
                        text: "\u00b0"
                    }

                    Label {
                        font {
                            pointSize: Application.font.pointSize * 1.1
                            capitalization: "AllUppercase"
                        }
                        text: `${temperatureUnitString}`
                    }
                }

                Item {
                    width: parent.width + rightUnitLbl.width + 8
                    height: parent.height + rightUnitLbl.height
                    anchors.verticalCenter: parent.verticalCenter

                    TapHandler {
                        onTapped: {
                            if (uiSession) {
                                uiSession.popupLayout.displayPopUp(tempUnitPop, true);
                            }
                        }
                    }
                }
            }

            Label {
                id: leftTempLabel
                anchors {
                    verticalCenter: parent.verticalCenter
                    verticalCenterOffset: labelVerticalOffset
                }
                font {
                    pointSize: _root.font.pointSize * 0.65
                }
                text: tempSliderDoubleHandle.first.value.toFixed(0)

                //! Unit
                Row {
                    id: leftUnitLbl
                    anchors.left: parent.right
                    anchors.top: parent.top
                    opacity: 0.6

                    Label {
                        font.pointSize: Application.font.pointSize * 0.8
                        text: "\u00b0"
                    }

                    Label {
                        font {
                            pointSize: Application.font.pointSize * 1.1
                            capitalization: "AllUppercase"
                        }
                        text: `${temperatureUnitString}`
                    }
                }

                Item {
                    width: parent.width + rightUnitLbl.width + 8
                    height: parent.height + rightUnitLbl.height
                    anchors.verticalCenter: parent.verticalCenter

                    TapHandler {
                        onTapped: {
                            if (uiSession) {
                                uiSession.popupLayout.displayPopUp(tempUnitPop, true);
                            }
                        }
                    }
                }
            }
        }

        //! Connections to sync slider with device.
        Connections {
            target: device
            enabled: !currentSchedule

            //! Update slider value (UI) with changed requestedTemp
            //! When setDesiredTemperature failed, update slider with previous value.
            function onRequestedTempChanged() {
                updateTemperatureValue(device.requestedTemp);
            }
        }

        Connections {
            target: _root

            //! Update slider value (UI) with changed TempratureUnit
            function onTemperatureUnitChanged() {
                updateTemperatureValue(currentSchedule?.temprature ?? (device?.requestedTemp ?? AppSpec.defaultRequestedTemperature));
            }
        }

        Connections {
            target: currentSchedule

            //! Update slider value (UI) with changed temperature in schedule
            function onMaximumTemperatureChanged() {
                autoModeTemperatureValueFromSchedule();
            }

            //! Update slider value (UI) with changed temperature in schedule
            function onMinimumTemperatureChanged() {
                autoModeTemperatureValueFromSchedule();
            }
        }
    }

    TempratureUnitPopup {
        id: tempUnitPop
        uiSession: _root.uiSession
    }

    /* States and Transitions
     * ************************************/
    state: {
        if (currentSchedule) return "auto-idle";

        if (device?.systemSetup?.systemMode === AppSpec.Auto) {
            if (tempSliderDoubleHandle.first.pressed && !tempSliderDoubleHandle.second.pressed) {
                return "auto-first-dragging";
            } else if (!tempSliderDoubleHandle.first.pressed && tempSliderDoubleHandle.second.pressed){
                return "auto-second-dragging";
            } else {
                return "auto-idle";
            }
        } else {
            if (_tempSlider.pressed) {
                return "non-auto-dragging";
            } else {
                return "non-auto-idle";
            }
        }
    }

    states: [
        State {
            name: "non-auto-idle"

            PropertyChanges {
                target: rightTempLabel
                x: (rightTempLabel.parent.width - rightTempLabel.width - rightUnitLbl.width) / 2
                visible: labelVisible
                opacity: 1
                text: Number(_tempSlider.value).toFixed(0)
            }

            PropertyChanges {
                target: leftTempLabel
                visible: false
            }

            PropertyChanges {
                target: _tempSlider
                visible: true
            }

            PropertyChanges {
                target: tempSliderDoubleHandle
                visible: false
                showGreySection: true
            }

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0
                y: -coolHeatLbl.height
            }
        },

        State {
            extend: "non-auto-idle"
            name: "non-auto-dragging"

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0.65
                y: leftTempLabel.y - coolHeatLbl.height
                text: device?.systemSetup?.systemMode === AppSpec.Heating ? "Heat to" : "Cool to"
            }
        },

        State {
            name: "auto-idle"

            PropertyChanges {
                target: rightTempLabel
                x: 5 * rightTempLabel.parent.width / 8 - rightUnitLbl.width / 2 - 4
                visible: labelVisible
                opacity: 1
                text: tempSliderDoubleHandle.second.value.toFixed(0)
            }

            PropertyChanges {
                target: leftTempLabel
                visible: labelVisible
                x: 2 * tempLabelParent.width / 8 - 8
            }

            PropertyChanges {
                target: _tempSlider
                visible: false
            }

            PropertyChanges {
                target: tempSliderDoubleHandle
                visible: true
                showGreySection: true
            }

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0
                y: coolHeatLbl.height
            }
        },

        State {
            name: "auto-first-dragging"

            PropertyChanges {
                target: rightTempLabel
                visible: false
            }

            PropertyChanges {
                target: leftTempLabel
                visible: labelVisible
                x: (leftTempLabel.parent.width - leftTempLabel.width - leftUnitLbl.width) / 2
            }

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0.65
                y: leftTempLabel.y - coolHeatLbl.height
                text: "Heat to"
            }

            PropertyChanges {
                target: tempSliderDoubleHandle
                showGreySection: false
            }

            PropertyChanges {
                target: _tempSlider
                visible: false
            }
        },

        State {
            name: "auto-second-dragging"

            PropertyChanges {
                target: rightTempLabel
                x: (rightTempLabel.parent.width - rightTempLabel.width - rightUnitLbl.width) / 2
                visible: true
                text: tempSliderDoubleHandle.second.value.toFixed(0)
            }

            PropertyChanges {
                target: leftTempLabel
                visible: false
            }

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0.65
                y: rightTempLabel.y - coolHeatLbl.height
                text: "Cool to"
            }

            PropertyChanges {
                target: _tempSlider
                visible: false
            }

            PropertyChanges {
                target: tempSliderDoubleHandle
                showGreySection: false
            }
        }
    ]

    transitions: [
        Transition {
            reversible: true
            from: "auto-idle"
            to: "auto-first-dragging,auto-second-dragging"

            SequentialAnimation {
                PropertyAnimation {
                    targets: [leftTempLabel, rightTempLabel]
                    property: "visible"
                    duration: 0
                }

                NumberAnimation {
                    targets: [leftTempLabel, rightTempLabel, coolHeatLbl]
                    properties: "x,y,opacity"
                }
            }
        }
    ]

    /* Functions
     * ****************************************************************************************/
    //! Update _tempSlider.value
    function updateTemperatureValue(temperature: real) {
        _tempSlider.value = Utils.convertedTemperatureClamped(temperature,
                                                              temperatureUnit,
                                                              minTemprature,
                                                              maxTemprature);
    }

    //! Update model based on _tempSlider value in heating/cooling mode.
    function updateTemperatureModel() {
        var celValue = (temperatureUnit === AppSpec.TempratureUnit.Cel)
                ? _tempSlider.value : Utils.fahrenheitToCelsius(_tempSlider.value);
        if (device && device.requestedTemp !== celValue) {
            deviceController.setDesiredTemperature(celValue);
            deviceController.updateEditMode(AppSpec.EMDesiredTemperature);
            deviceController.saveSettings();
        }
    }

    //! Update tempSliderDoubleHandle values
    function autoModeTemperatureValueFromSchedule() {
        if (currentSchedule) {
            // TODO: Check for clamping data
            tempSliderDoubleHandle.first.value = Utils.convertedTemperature(currentSchedule.minimumTemperature, temperatureUnit);
            tempSliderDoubleHandle.second.value = Utils.convertedTemperature(currentSchedule.maximumTemperature, temperatureUnit);
        }
    }
}
