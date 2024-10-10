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
    readonly property real  minimumTemperature: {
        var temperatureValue = _tempSlider.first.value;
        if (schedule.systemMode === AppSpec.Heating) {
            temperatureValue = singleTemperatureSlider.control.value;
        }

        return (isCelcius ? temperatureValue : Utils.fahrenheitToCelsius(temperatureValue));
    }

    readonly property real  maximumTemperature: {
        var temperatureValue = _tempSlider.second.value;
        if (schedule.systemMode === AppSpec.Cooling) {
            temperatureValue = singleTemperatureSlider.control.value;

        }

        return (isCelcius ? temperatureValue : Utils.fahrenheitToCelsius(temperatureValue))
    }



    //! Minimum temperature
    property real               minTemperature: {
        var minTemp;

        if (schedule.systemMode === AppSpec.Cooling) {
            minTemp = isCelcius ? Utils.fahrenheitToCelsius(AppSpec.minimumCoolingTemperatiureF) : AppSpec.minimumCoolingTemperatiureF;

        } else if (schedule.systemMode === AppSpec.Heating) {
            minTemp = isCelcius ? Utils.fahrenheitToCelsius(AppSpec.minimumHeatingTemperatiureF) : AppSpec.minimumHeatingTemperatiureF;

        } else {
            minTemp = isCelcius ? AppSpec.autoMinimumTemperatureC : AppSpec.autoMinimumTemperatureF;
        }

        return minTemp;
    }

    //! Maximum temperature
    property real               maxTemperature: {
        var maxTemp;

        if (schedule.systemMode === AppSpec.Cooling) {
            maxTemp = isCelcius ? Utils.fahrenheitToCelsius(AppSpec.maximumCoolingTemperatiureF) : AppSpec.maximumCoolingTemperatiureF;

        } else if (schedule.systemMode === AppSpec.Heating) {
            maxTemp = isCelcius ? Utils.fahrenheitToCelsius(AppSpec.maximumHeatingTemperatiureF) : AppSpec.maximumHeatingTemperatiureF;

        } else {
            maxTemp = isCelcius ? AppSpec.autoMaximumTemperatureC : AppSpec.autoMaximumTemperatureF;
        }

        return maxTemp;
    }



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
            save();
            backButtonCallback();
        }
    }

    // Use in the Auto mode
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9

        visible: schedule.systemMode !== AppSpec.Heating && schedule.systemMode !== AppSpec.Cooling
        spacing: 16

        RowLayout {
            Layout.preferredWidth: parent.width * 0.9
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 100

            spacing: 25

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
            Layout.topMargin: 40

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


    //! Use in the heating/cooling mode
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        spacing: 60

        visible: schedule.systemMode === AppSpec.Heating || schedule.systemMode === AppSpec.Cooling

        SingleTemperatureSlider {
            id: singleTemperatureSlider

            Layout.fillWidth: true

            leftSideColor:  "#ea0600"
            rightSideColor: "#0097cd"
            labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(deviceController.temperatureUnit))
            control.from: minTemperature;
            control.to: maxTemperature;
        }

        Label {
            Layout.preferredWidth: parent.width

            text: "<b>" + (schedule.systemMode === AppSpec.Heating ? "Heating" : "Cooling") + " schedule</b> - to set the " +
                  (schedule.systemMode === AppSpec.Heating ? "heat" : "cool") + " setpoint for the selected period"
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            horizontalAlignment: Qt.AlignLeft
            font.pointSize: root.font.pointSize * 0.7
        }
    }

    /* Functions
     * ****************************************************************************************/

    function updateSliderValues() {
        if (schedule.systemMode === AppSpec.Heating) {
            singleTemperatureSlider.control.value = Utils.convertedTemperatureClamped(schedule?.minimumTemperature ?? singleTemperatureSlider.control.from, temperatureUnit,
                                                                                      minTemperature, maxTemperature);

        } else if (schedule.systemMode === AppSpec.Cooling) {
            singleTemperatureSlider.control.value = Utils.convertedTemperatureClamped(schedule?.maximumTemperature ?? singleTemperatureSlider.control.from, temperatureUnit,
                                                                                      minTemperature, maxTemperature);

        } else {
            //! Create schedule in auto and off mode.
            _tempSlider.first.value = Utils.convertedTemperatureClamped(schedule?.minimumTemperature ?? _tempSlider.from, temperatureUnit,
                                                                        minTemperature, maxTemperature - _tempSlider.difference);

            _tempSlider.second.value = Utils.convertedTemperatureClamped(schedule?.maximumTemperature ?? _tempSlider.to, temperatureUnit,
                                                                         _tempSlider.first.value + _tempSlider.difference, maxTemperature);
        }
    }

    //! Save the schedule temperature
    function save() {
        if (schedule) {
            var minTemperature = _tempSlider.first.value;
            var maxTemperature = _tempSlider.second.value;

            if (schedule.systemMode === AppSpec.Heating) {
                // Update minimum temperature as heating temperature
                minTemperature = singleTemperatureSlider.control.value;

            } else if (schedule.systemMode === AppSpec.Cooling) {
                // Update maximum temperature as cooling temperature
                maxTemperature = singleTemperatureSlider.control.value;
            }

            // Save temperatures as celcius.
            schedule.minimumTemperature = isCelcius ? minTemperature : Utils.fahrenheitToCelsius(minTemperature);
            schedule.maximumTemperature = isCelcius ? maxTemperature : Utils.fahrenheitToCelsius(maxTemperature);

        } else {
            console.log("Schedule temperature page: Schedule is undefined, so we can not save it.")
        }
    }
}
