import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 *  ScheduleTempraturePage is a page for setting temprature in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property ScheduleCPP    schedule

    //! Temprature is alwasy valid
    readonly property bool  isValid: true

    property bool editMode: false

    property int temperatureUnit:      appModel?.setting?.tempratureUnit ?? AppSpec.defaultTemperatureUnit

    //! Is Celsius selected as the unit?
    readonly property bool           isCelcius:  temperatureUnit === AppSpec.TempratureUnit.Cel

    //! Temperature value: this is always in celsius
    readonly property real  minimumTemperature: (isCelcius ? _tempSlider.first.value : Utils.fahrenheitToCelsius(_tempSlider.first.value))
    readonly property real  maximumTemperature: (isCelcius ? _tempSlider.second.value : Utils.fahrenheitToCelsius(_tempSlider.second.value))

    //! Minimum temperature
    property real               minTemperature: isCelcius ? AppSpec.autoMinimumTemperatureC : AppSpec.autoMinimumTemperatureF

    //! Maximum temperature
    property real               maxTemperature: isCelcius ? AppSpec.autoMaximumTemperatureC : AppSpec.autoMaximumTemperatureF


    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    leftPadding: 8 * scaleFactor
    rightPadding: 8 * scaleFactor
    title: "Temperature (\u00b0" + (AppSpec.temperatureUnitString(temperatureUnit)) + ")"
    backButtonVisible: false
    titleHeadeingLevel: 4

    Component.onCompleted: {
        updateSliderValues();
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
        visible: editMode
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            if (schedule) {
                if (temperatureUnit === AppSpec.TempratureUnit.Cel) {
                    schedule.minimumTemperature = _tempSlider.first.value;
                    schedule.maximumTemperature = _tempSlider.second.value;

                } else {
                    schedule.minimumTemperature = Utils.fahrenheitToCelsius(_tempSlider.first.value);
                    schedule.maximumTemperature = Utils.fahrenheitToCelsius(_tempSlider.second.value);
                }
            }

            backButtonCallback();
        }
    }

    RowLayout {
        anchors.centerIn: parent
        width: parent.width * 0.85

        spacing: 20

        //! Temperature icon
        RoniaTextIcon {
            Layout.leftMargin: 24
            font.pointSize: _root.font.pointSize * 2
            text: "\uf2c8" //! temperature-three-quarters icon
        }

        TemperatureFlatRangeSlider {
            id: _tempSlider

            Layout.fillWidth: true

            from: minTemperature
            to: maxTemperature

            difference: temperatureUnit === AppSpec.TempratureUnit.Fah ? AppSpec.autoModeDiffrenceF : AppSpec.autoModeDiffrenceC

            first.onPressedChanged: {
                if (deviceController && !first.pressed) {
                    schedule.minimumTemperature = (temperatureUnit === AppSpec.TempratureUnit.Fah
                                                       ? Utils.fahrenheitToCelsius(first.value)
                                                       : first.value);
                    deviceController.saveSettings();
                }
            }

            second.onPressedChanged: {
                if (deviceController && !second.pressed) {
                    schedule.maximumTemperature = (temperatureUnit === AppSpec.TempratureUnit.Fah
                                                       ? Utils.fahrenheitToCelsius(second.value)
                                                       : second.value);
                    deviceController.saveSettings();
                }
            }

            labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(temperatureUnit))
            fromValueCeil: Utils.convertedTemperature(AppSpec.maxAutoMinTemp, temperatureUnit)
            toValueFloor: Utils.convertedTemperature(AppSpec.minAutoMaxTemp, temperatureUnit)
        }
    }

    function updateSliderValues() {
        _tempSlider.first.value = Utils.convertedTemperatureClamped(schedule?.minimumTemperature ?? _tempSlider.from, temperatureUnit,
                                                       minTemperature, maxTemperature - _tempSlider.difference)

        _tempSlider.second.value = Utils.convertedTemperatureClamped(schedule?.maximumTemperature ?? _tempSlider.to, temperatureUnit,
                                                        _tempSlider.first.value + _tempSlider.difference, maxTemperature)

    }
}
