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

    //! Unit of temprature
    property string             unit: (device?.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C") ?? "F"

    //! Minimum temprature
    property real               minTemprature: device?._minimumTemperature ?? 40

    //! Maximum temprature
    property real               maxTemprature: device?._maximumTemperature ?? 90

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


    onDraggingChanged: {
        if (dragging)
            deviceController.updateEditMode(AppSpec.EMDesiredTemperature);
        else
            deviceController.updateEditMode(AppSpec.EMNone);
    }

    /* Object properties
     * ****************************************************************************************/
    onCurrentScheduleChanged: {
        if (currentSchedule) {
            _tempSlider.value = Utils.convertedTemperatureClamped(currentSchedule.temprature,
                                                                  device.setting.tempratureUnit);
        } else if (device) {
            _tempSlider.value = Utils.convertedTemperatureClamped(device.requestedTemp,
                                                                  device.setting.tempratureUnit);
        }
    }

    font.pointSize: Qt.application.font.pointSize * 2.8
    background: null
    contentItem: Item {
        SemiCircleSlider {
            id: _tempSlider
            anchors.centerIn: parent
            width: parent.width
            height: width / 2
            visible: !tempSliderDoubleHandle.visible
            enabled: labelVisible && !currentSchedule
            from: minTemprature
            to: maxTemprature

            //! Note: this binding will be broken use Connections instead
            value: Utils.convertedTemperatureClamped(currentSchedule?.temprature ?? (device?.requestedTemp ?? 18.0), device.setting.tempratureUnit)

            //! Use onPressed instead of on value changed so value is only applied to device when
            //! Dragging is finished
            onPressedChanged: {
                if (!pressed) {
                    var celValue = (device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah)
                            ? Utils.fahrenheitToCelsius(value) : value;
                    if (device && device.requestedTemp !== celValue) {
                        uiSession.deviceController.setDesiredTemperature(celValue);
                        deviceController.pushSettings();
                    }
                }
            }
        }

        //! Double handle semi circle slider
        SemiCircleSliderDoubleHandle {
            id: tempSliderDoubleHandle
            anchors.centerIn: parent
            width: parent.width
            height: width / 2
            visible: device?.systemSetup?.systemMode === AppSpec.Auto
            enabled: labelVisible && !currentSchedule
            difference: device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? 4 : 2.5

            maxAutoMinTemp: Utils.convertedTemperature(AppSpec.maxAutoMinTemp, device?.setting?.tempratureUnit ?? AppSpec.TempratureUnit.Fah)
            minAutoMaxTemp: Utils.convertedTemperature(AppSpec.minAutoMaxTemp, device?.setting?.tempratureUnit ?? AppSpec.TempratureUnit.Fah)

            from: minTemprature
            to: maxTemprature

            onVisibleChanged: {
                if (visible) {
                    //! Set difference
                    tempSliderDoubleHandle.difference = device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? 4 : 2.5
                    tempSliderDoubleHandle.updateFirstSecondValues();
                }
            }

            first.onPressedChanged: {
                if (deviceController && !first.pressed) {
                    deviceController.setAutoMinReqTemp(device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                       ? Utils.fahrenheitToCelsius(first.value)
                                                       : first.value);
                }
            }

            second.onPressedChanged: {
                if (deviceController && !second.pressed) {
                    deviceController.setAutoMaxReqTemp(device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                                                       ? Utils.fahrenheitToCelsius(second.value)
                                                       : second.value);
                }
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

                function onTempratureUnitChanged() { updateFirstSecondValuesTmr.restart(); }
            }

            Connections {
                target: _root
                enabled: tempSliderDoubleHandle.visible

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
                    tempSliderDoubleHandle.difference = device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? 4 : 2.5
                    tempSliderDoubleHandle.updateFirstSecondValues();
                }
            }

            function updateFirstValue()
            {
                if (!device) return;

                first.value = Utils.convertedTemperatureClamped(device.autoMinReqTemp,
                                                                device.setting.tempratureUnit,
                                                                minTemprature,
                                                                maxTemprature);
            }

            function updateSecondValue()
            {
                if (!device) return;

                second.value = Utils.convertedTemperatureClamped(device.autoMaxReqTemp,
                                                                 device.setting.tempratureUnit,
                                                                 minTemprature,
                                                                 maxTemprature)
            }

            function updateFirstSecondValues()
            {
                if (!device) return;

                //! First calculate new values for handles without setting them
                const firstValue = Utils.convertedTemperatureClamped(device.autoMinReqTemp,
                                                                     device.setting.tempratureUnit,
                                                                     minTemprature,
                                                                     maxTemprature);
                const secondValue = Utils.convertedTemperatureClamped(device.autoMaxReqTemp,
                                                                      device.setting.tempratureUnit,
                                                                      minTemprature,
                                                                      maxTemprature);

                //! Now first set first.maxValue and second.minValue then update their actual values
                first.setMaxValue(secondValue - tempSliderDoubleHandle.difference);
                second.setMinValue(firstValue + tempSliderDoubleHandle.difference);

                first.value = firstValue;
                second.value = secondValue;
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
            Label {
                id: rightUnitLbl
                anchors.left: parent.right
                anchors.top: parent.top
                opacity: 0.6
                font {
                    pointSize: _root.font.pointSize / 2
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
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
            Label {
                id: leftUnitLbl
                anchors.left: parent.right
                anchors.top: parent.top
                opacity: 0.6
                font {
                    pointSize: _root.font.pointSize / 2
                    capitalization: "AllUppercase"
                }
                text: `\u00b0${unit}`
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

        //! Connections to sync slider with device.
        Connections {
            target: device
            enabled: !currentSchedule

            //! Update slider value (UI) with changed requestedTemp
            //! When setDesiredTemperature failed, update slider with previous value.
            function onRequestedTempChanged() {
                _tempSlider.value = Utils.convertedTemperatureClamped(device.requestedTemp,
                                                                      device.setting.tempratureUnit);
            }
        }

        Connections {
            target: _root

            //! Update slider value (UI) with changed TempratureUnit
            function onUnitChanged() {
                _tempSlider.value = Utils.convertedTemperatureClamped(currentSchedule?.temprature ?? device.requestedTemp,
                                                                      device.setting.tempratureUnit);
            }
        }

        Connections {
            target: currentSchedule

            //! Update slider value (UI) with changed temperature in schedule
            function onTempratureChanged() {
                _tempSlider.value = Utils.convertedTemperatureClamped(currentSchedule.temprature,
                                                                      device.setting.tempratureUnit);
            }
        }
    }

    TempratureUnitPopup {
        id: tempUnitPop
        uiSession: _root.uiSession
    }

    /* States and Transitions
     * ************************************/
    state: "non-auto-idle"
    states: [
        State {
            when: device?.systemSetup?.systemMode !== AppSpec.Auto && !_tempSlider.pressed
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
            }

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0
                y: coolHeatLbl.height
            }
        },

        State {
            extend: "non-auto-idle"
            when: device?.systemSetup?.systemMode !== AppSpec.Auto && _tempSlider.pressed
            name: "non-auto-dragging"
        },

        State {
            when: device?.systemSetup?.systemMode === AppSpec.Auto
                  && !tempSliderDoubleHandle.first.pressed && !tempSliderDoubleHandle.second.pressed
            name: "auto-idle"

            PropertyChanges {
                target: rightTempLabel
                x: 3 * rightTempLabel.parent.width / 5 - 16
                visible: labelVisible
                opacity: 1
                text: tempSliderDoubleHandle.second.value.toFixed(0)
            }

            PropertyChanges {
                target: leftTempLabel
                visible: labelVisible
                x: leftTempLabel.parent.width / 4
            }

            PropertyChanges {
                target: _tempSlider
                visible: false
            }

            PropertyChanges {
                target: tempSliderDoubleHandle
                visible: true
            }

            PropertyChanges {
                target: coolHeatLbl
                opacity: 0
                y: coolHeatLbl.height
            }
        },

        State {
            when: device?.systemSetup?.systemMode === AppSpec.Auto
                  && tempSliderDoubleHandle.first.pressed && !tempSliderDoubleHandle.second.pressed
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
                text: "Cool to"
            }
        },

        State {
            when: device?.systemSetup?.systemMode === AppSpec.Auto
                  && tempSliderDoubleHandle.second.pressed && !tempSliderDoubleHandle.first.pressed
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
                text: "Heat to"
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
}
