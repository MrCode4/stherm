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
    readonly property alias     dragging: _tempSlider.pressed

    //! Visibility of temprature label
    property bool               labelVisible: true

    //! Label width
    readonly property alias     labelWidth: firstTempLabel.width


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

            first.value: device ? Utils.convertedTemperatureClamped(device.autoMinReqTemp,
                                                                    device.setting.tempratureUnit,
                                                                    minTemprature,
                                                                    maxTemprature)
                                : 60
            second.value: device ? Utils.convertedTemperatureClamped(device.autoMaxReqTemp,
                                                                     device.setting.tempratureUnit,
                                                                     minTemprature,
                                                                     maxTemprature)
                                 : 80

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
        }

        //! Label to show desired temperature in cooling/heating mode and second temperature in auto
        Label {
            id: firstTempLabel
            visible: labelVisible
            x: device?.systemSetup?.systemMode !== AppSpec.Auto ? (parent.width - width) / 2 : parent.width / 2 + 30
            anchors {
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: labelVerticalOffset
            }
            font.pointSize: _root.font.pointSize * 0.8
            text: _tempSlider.visible ? Number(_tempSlider.value).toLocaleString(locale, "f", 0)
                                      : tempSliderDoubleHandle.second.value.toFixed(0)

            //! Unit
            Label {
                id: unitLbl
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
                width: parent.width + unitLbl.width + 8
                height: parent.height + unitLbl.height
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
                SequentialAnimation {
                    PauseAnimation { duration: 150 }
                    NumberAnimation { duration: 250 }
                }
            }
        }

        Label {
            id: secondTempLabel
            visible: labelVisible && tempSliderDoubleHandle.visible
            x: device?.systemSetup?.systemMode === AppSpec.Auto ? parent.width / 2 - width - unitLbl.width - 30 : (parent.width - width) / 2
            anchors {
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: labelVerticalOffset
            }
            font.pointSize: _root.font.pointSize * 0.8
            text: tempSliderDoubleHandle.first.value.toFixed(0)

            //! Unit
            Label {
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
                width: parent.width + unitLbl.width + 8
                height: parent.height + unitLbl.height
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
                SequentialAnimation {
                    PauseAnimation { duration: 150 }
                    NumberAnimation { duration: 250 }
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
}
