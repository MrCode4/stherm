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
    property string             unit: (device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah ? "F" : "C") ?? "F"

    //! Minimum temprature
    property real               minTemprature: Utils.convertedTemperature(AppSpec.minimumTemperatureC, device.setting.tempratureUnit);

    //! Maximum temprature
    property real               maxTemprature: Utils.convertedTemperature(AppSpec.maximumTemperatureC, device.setting.tempratureUnit);

    //! Offset of desired temp label
    property int                labelVerticalOffset: -8

    //! Holds whether SemiCircleSlider is being dragged
    readonly property alias     dragging: _tempSlider.pressed

    //! Visibility of temprature label
    property bool               labelVisible: true

    //! Label width
    readonly property alias     labelWidth: _desiredTempratureLbl.width

    /* Object properties
     * ****************************************************************************************/
    onCurrentScheduleChanged: {
        if (currentSchedule) {
            _tempSlider.value = Utils.convertedTemperature(currentSchedule.temprature,
                                                           device.setting.tempratureUnit);
        } else if (device) {
            _tempSlider.value = Utils.convertedTemperature(device.requestedTemp,
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
            enabled: labelVisible && !currentSchedule
            from: minTemprature
            to: maxTemprature

            //! Note: this binding will be broken use Connections instead
            value: Utils.convertedTemperature(currentSchedule?.temprature ?? (device?.requestedTemp ?? 18.0), device.setting.tempratureUnit)

            //! Use onPressed instead of on value changed so value is only applied to device when
            //! Dragging is finished
            onPressedChanged: {
                if (!pressed) {
                    var celValue = (device.setting.tempratureUnit === AppSpec.TempratureUnit.Fah)
                            ? Utils.fahrenheitToCelsius(value) : value;
                    if (device && device.requestedTemp !== celValue) {
                        uiSession.deviceController.setDesiredTemperature(celValue);
                    }
                }
            }
        }

        //! Desired temprature label
        Label {
            id: _desiredTempratureLbl
            visible: labelVisible
            anchors.centerIn: parent
            anchors.verticalCenterOffset: labelVerticalOffset
            text: Number(_tempSlider.value).toLocaleString(locale, "f", 0)

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
        }

        //! Connections to sync slider with device.
        Connections {
            target: device
            enabled: !currentSchedule

            //! Update slider value (UI) with changed requestedTemp
            //! When setDesiredTemperature failed, update slider with previous value.
            function onRequestedTempChanged() {
                _tempSlider.value = Utils.convertedTemperature(device.requestedTemp,
                                                               device.setting.tempratureUnit);
            }            
        }

        Connections {
            target: _root

            //! Update slider value (UI) with changed TempratureUnit
            function onUnitChanged() {
                _tempSlider.value = Utils.convertedTemperature(currentSchedule?.temprature ?? device.requestedTemp,
                                                               device.setting.tempratureUnit);
            }
        }

        Connections {
            target: currentSchedule

            //! Update slider value (UI) with changed temperature in schedule
            function onTempratureChanged() {
                _tempSlider.value = Utils.convertedTemperature(currentSchedule.temprature,
                                                               device.setting.tempratureUnit);
            }
        }
    }

    TempratureUnitPopup {
        id: tempUnitPop
        uiSession: _root.uiSession
    }
}
