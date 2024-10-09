import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 *  ScheduleTempraturePage is a page for setting temprature in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: root

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
        parent: schedule ? root.header.contentItem : root
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

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9


        spacing: 40

        RowLayout {
            spacing: 25
            Layout.preferredWidth: parent.width * 0.9
            Layout.alignment: Qt.AlignHCenter

            //! Temperature icon
            RoniaTextIcon {
                Layout.leftMargin: 5
                font.pointSize: root.font.pointSize * 1.5
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
                leftLabelPrefix:  "<p style='color:#ea0600; font-size:12px;'>Heat to</p>"
                rightLabelPrefix: "<p style='color:#0097cd; font-size:12px;'>Cool to</p>"
                fromValueCeil: Utils.convertedTemperature(AppSpec.maxAutoMinTemp, temperatureUnit)
                toValueFloor: Utils.convertedTemperature(AppSpec.minAutoMaxTemp, temperatureUnit)
            }
        }

        RowLayout {
            spacing: 20
            Layout.preferredWidth: parent.width
            Layout.topMargin: 20

            Label {
                Layout.preferredWidth: parent.width / 2 - 5

                text: "<span style='color:#ea0600;'>Heat to</span> - Heating will be turned off when the indoor temperature is above this value"
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                horizontalAlignment: Qt.AlignLeft
                font.pointSize: root.font.pointSize * 0.7
            }

            Label {
                Layout.preferredWidth: parent.width / 2 - 5

                text: "<span style='color:#0097cd;'>Cool to</span> - Cooling will be turned off when the indoor temperature is below this value"
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                horizontalAlignment: Qt.AlignLeft
                font.pointSize: root.font.pointSize * 0.7
            }
        }

        Label {
            Layout.preferredWidth: parent.width

            horizontalAlignment: Qt.AlignHCenter
            wrapMode: Text.WordWrap
            font.pointSize: root.font.pointSize * 0.7
            text: "After activation the schedule will override the current Auto mode temperature settings."
        }
    }


    /* Functions
     * ****************************************************************************************/

    function updateSliderValues() {
        _tempSlider.first.value = Utils.convertedTemperatureClamped(schedule?.minimumTemperature ?? _tempSlider.from, temperatureUnit,
                                                       minTemperature, maxTemperature - _tempSlider.difference)

        _tempSlider.second.value = Utils.convertedTemperatureClamped(schedule?.maximumTemperature ?? _tempSlider.to, temperatureUnit,
                                                        _tempSlider.first.value + _tempSlider.difference, maxTemperature)

    }
}
