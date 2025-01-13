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
        Qt.callLater(updateUITemperature);
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
                updateUITemperature();
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
                    updateUITemperature();
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
                enabled: tempSliderDoubleHandle.visible && !currentSchedule

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

                //! Update
                function onTempratureUnitChanged() {
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

                    updateUITemperature();
                }
            }

            function updateAutoMinReqTempModel() {
                if (deviceController && !first.pressed) {
                    let value = first.value;
                    deviceController.setAutoMinReqTemp(value);
                }
            }

            function updateAutoMaxReqTempModel() {
                if (deviceController && !second.pressed) {
                    let value = second.value;
                    deviceController.setAutoMaxReqTemp(value);
                }
            }

            function updateFirstValue()
            {
                if (!device || currentSchedule) return;

                let value = Utils.convertedTemperatureClamped(device.autoMinReqTemp,
                                                              temperatureUnit,
                                                              minTemprature,
                                                              maxTemprature);

                first.value = value;
            }

            function updateSecondValue()
            {
                if (!device || currentSchedule) return;

                let value = Utils.convertedTemperatureClamped(device.autoMaxReqTemp,
                                                              temperatureUnit,
                                                              minTemprature,
                                                              maxTemprature);
                second.value = value;
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

                if (first.value !== firstValue) {
                    first.value = firstValue;
                    updateAutoMinReqTempModel();
                }

                if (second.value !== secondValue) {
                    second.value = secondValue;
                    updateAutoMaxReqTempModel();
                }
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
                    temp = _tempSlider.value;

                } else if (tempSliderDoubleHandle.first.pressed) {
                    temp = tempSliderDoubleHandle.first.value;

                } else if (tempSliderDoubleHandle.second.pressed) {
                    temp = tempSliderDoubleHandle.second.value;
                }

                temp = AppUtilities.getTruncatedvalue(temp);

                // The slider value is currently converted to the selected unit.
                // However, TemperatureLabel needs Celsius and will convert to selected unit itself.
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

            // To fit label and schedule in home screen
            anchors.verticalCenterOffset: (currentSchedule && device.systemSetup.systemMode !== AppSpec.Auto) ? -15 : 0


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
                text: AppUtilities.getTruncatedvalue(tempSliderDoubleHandle.first.value)

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
                updateUITemperature();
            }
        }

        Connections {
            target: _root

            //! Update slider value (UI) with changed TempratureUnit
            function onTemperatureUnitChanged() {
                updateUITemperature();
            }
        }

        Connections {
            target: currentSchedule

            //! Update slider value (UI) with changed temperature in schedule
            function onMaximumTemperatureChanged() {
                updateUITemperature();
            }

            //! Update slider value (UI) with changed temperature in schedule
            function onMinimumTemperatureChanged() {
                updateUITemperature();
            }
        }

        Connections {
            target: device?.systemSetup ?? null

            //! Update slider value (UI) with mode change.
            function onSystemModeChanged() {
                updateUITemperature();
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
        if (currentSchedule) {
            if (device?.systemSetup?.systemMode === AppSpec.Auto) {
                return "auto-idle";

            } else {
                return "non-auto-idle";
            }
        }


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
                text: AppUtilities.getTruncatedvalue(_tempSlider.value)
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
                text: AppUtilities.getTruncatedvalue(tempSliderDoubleHandle.second.value)
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
                text: AppUtilities.getTruncatedvalue(tempSliderDoubleHandle.second.value)
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
        var value = Utils.convertedTemperatureClamped(temperature,
                                                      temperatureUnit,
                                                      minTemprature,
                                                      maxTemprature);
        _tempSlider.value = (value);
    }

    //! Update model based on _tempSlider value in heating/cooling mode.
    function updateTemperatureModel() {
        deviceController.setDesiredTemperature(_tempSlider.value);
    }

    //! Update tempSliderDoubleHandle values
    function updateAutoModeTemperatureValueFromSchedule() {
        if (currentSchedule) {
            // TODO: Check for clamping data
            var tempValue = Utils.convertedTemperatureClamped(currentSchedule.minimumTemperature, temperatureUnit, minTemprature, maxTemprature);
            tempSliderDoubleHandle.first.value = tempValue;

            tempValue = Utils.convertedTemperatureClamped(currentSchedule.maximumTemperature, temperatureUnit, minTemprature, maxTemprature);
            tempSliderDoubleHandle.second.value =  tempValue;
        }
    }

    // The auto mode slider (tempSliderDoubleHandle) should be updated when
    // a schedule is defined or becomes null.
    // If a schedule exists, update the slider with
    // the schedule's minimum and maximum temperature values based on the current mode.
    // If no schedule is defined, update each visible slider with the device's values.
    function updateUITemperature() {
        if (currentSchedule) {
            if (device?.systemSetup?.systemMode === AppSpec.Auto) {
                updateAutoModeTemperatureValueFromSchedule();

            } else {
                updateTemperatureValue(currentSchedule.effectiveTemperature(device?.systemSetup?.systemMode ?? AppSpec.Off));
            }

        } else {
            if (device?.systemSetup?.systemMode === AppSpec.Auto)
                tempSliderDoubleHandle.updateFirstSecondValues();

            else
                updateTemperatureValue(device?.requestedTemp ?? AppSpec.defaultRequestedTemperature);

        }
    }
}
