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
            from: minTemprature
            to: maxTemprature

            first.onPressedChanged: {
                if (!device) return;

                if (!first.pressed && first.value.toFixed(2) !== device.autoMinReqTemp.toFixed(2)) {
                    device.autoMinReqTemp = device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                            ? Utils.fahrenheitToCelsius(first.value)
                            : first.value
                }
            }

            second.onPressedChanged: {
                if (!device) return;

                if (!second.pressed && second.value.toFixed(2) !== device.autoMaxReqTemp.toFixed(2)) {
                    device.autoMaxReqTemp = device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah
                            ? Utils.fahrenheitToCelsius(second.value)
                            : second.value
                }
            }

            //! Use Connections to update first and second values
            Connections {
                target: device ?? null

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

                function onTempratureUnitChanged()
                {
                    tempSliderDoubleHandle.updateFirstSeconValues();
                }
            }

            Connections {
                target: _root

                function onMinTempratureChanged()
                {
                    tempSliderDoubleHandle.updateFirstSecondValues();
                }

                function onMaxTempratureChanged()
                {
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

            function updateFirstSeconValues()
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

        //! Label to show desired temperature in cooling/heating mode and second temperature in auto
        Label {
            id: rightTempLabel
            visible: labelVisible
            x: device?.systemSetup?.systemMode === AppSpec.Auto
               ? 3 * parent.width / 4 - width - rightUnitLbl.width + 4
               : (parent.width - width) / 2
            opacity: tempSliderDoubleHandle.first.pressed ? 0 : 1.
            anchors {
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: labelVerticalOffset
            }
            font {
                pointSize: _root.font.pointSize * 0.65
            }
            text: _tempSlider.visible ? Number(_tempSlider.value).toLocaleString(locale, "f", 0)
                                      : tempSliderDoubleHandle.second.value.toFixed(0)

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

            Behavior on x {
                enabled: _root.enableAnimations
                SequentialAnimation {
                    PauseAnimation { duration: 150 }
                    NumberAnimation { duration: 250 }
                }
            }
            Behavior on opacity { NumberAnimation { duration: 100 } }
        }

        Label {
            id: leftTempLabel
            visible: labelVisible && tempSliderDoubleHandle.visible
            x: device?.systemSetup?.systemMode === AppSpec.Auto
               ? parent.width / 4
               : (parent.width - width) / 2
            opacity: tempSliderDoubleHandle.second.pressed ? 0 : 1.
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

            Behavior on x {
                enabled: _root.enableAnimations
                SequentialAnimation {
                    PauseAnimation { duration: 150 }
                    NumberAnimation { duration: 250 }
                }
            }
            Behavior on opacity { NumberAnimation { duration: 100 } }
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
}
