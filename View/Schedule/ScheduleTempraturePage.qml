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

    readonly property bool           isHeating:   schedule.systemMode === AppSpec.Heating || schedule.systemMode === AppSpec.EmergencyHeat
    readonly property bool           isCooling:   schedule.systemMode === AppSpec.Cooling

    //! Temperature value: this is always in celsius
    readonly property real  minimumTemperature: {
        var temperatureValue = _tempSlider.first.value;
        if (isHeating) {
            temperatureValue = singleTemperatureSlider.control.value;
        }

        return (isCelcius ? temperatureValue : Utils.fahrenheitToCelsius(temperatureValue));
    }

    readonly property real  maximumTemperature: {
        var temperatureValue = _tempSlider.second.value;
        if (isCooling) {
            temperatureValue = singleTemperatureSlider.control.value;

        }

        return (isCelcius ? temperatureValue : Utils.fahrenheitToCelsius(temperatureValue))
    }



    //! Minimum temperature
    property real               minTemperature: {
        var minTemp;

        if (isCooling) {
            minTemp = isCelcius ? AppSpec.minimumCoolingTemperatiureC : AppSpec.minimumCoolingTemperatiureF;

        } else if (isHeating) {
            minTemp = isCelcius ? Utils.fahrenheitToCelsius(AppSpec.minimumHeatingTemperatiureF) : AppSpec.minimumHeatingTemperatiureF;

        } else {
            minTemp = isCelcius ? AppSpec.autoMinimumTemperatureC : AppSpec.autoMinimumTemperatureF;
        }

        return minTemp;
    }

    //! Maximum temperature
    property real               maxTemperature: {
        var maxTemp;

        if (isCooling) {
            maxTemp = isCelcius ? AppSpec.maximumCoolingTemperatiureC : AppSpec.maximumCoolingTemperatiureF;

        } else if (isHeating) {
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

        visible: !isHeating && !isCooling
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
                text: FAIcons.temperatureThreeQuarters
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
                font.pointSize: root.font.pointSize * 0.67
            }

            Label {
                Layout.preferredWidth: parent.width / 2 - 5

                text: "<span style='color:#0097cd;'>Cool to</span> - Cooling will be turned off when the indoor temperature is below this value"
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                horizontalAlignment: Qt.AlignLeft
                font.pointSize: root.font.pointSize * 0.67
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

        visible: isHeating || isCooling

        SingleIconSlider {
            id: singleTemperatureSlider

            Layout.fillWidth: true

            icon: FAIcons.temperatureThreeQuarters
            leftSideColor:  "#ea0600"
            rightSideColor: "#0097cd"
            labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(deviceController.temperatureUnit))
            from: AppUtilities.getTruncatedvalue(minTemperature);
            to: maxTemperature;
        }

        Label {
            Layout.preferredWidth: parent.width

            text: "<b>" + (isHeating ? "Heating" : "Cooling") + " schedule</b> - to set the " +
                  (isHeating ? "heat" : "cool") + " setpoint for the selected period"
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            horizontalAlignment: Qt.AlignLeft
            font.pointSize: root.font.pointSize * 0.7
        }
    }

    /* Functions
     * ****************************************************************************************/

    function updateSliderValues() {
        if (isHeating) {
            singleTemperatureSlider.control.value = Utils.convertedTemperatureClamped(schedule?.minimumTemperature ?? singleTemperatureSlider.control.from, temperatureUnit,
                                                                                      minTemperature, maxTemperature);

        } else if (isCooling) {
            var value = Utils.convertedTemperatureClamped(schedule?.maximumTemperature ?? singleTemperatureSlider.control.from, temperatureUnit,
                                                                                      minTemperature, maxTemperature);

            singleTemperatureSlider.control.value = AppUtilities.getTruncatedvalue(value);

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

            if (isHeating) {
                // Update minimum temperature as heating temperature
                minTemperature = singleTemperatureSlider.control.value;

            } else if (isCooling) {
                // Update maximum temperature as cooling temperature
                maxTemperature = singleTemperatureSlider.control.value;
            }

            console.log("MAK In schedule maxTemperature", maxTemperature)
            console.log("MAK In schedule minTemperature", minTemperature)

            // Save temperatures as celcius.
            schedule.minimumTemperature = isCelcius ? minTemperature : Utils.fahrenheitToCelsius(minTemperature);
            schedule.maximumTemperature = isCelcius ? maxTemperature : Utils.fahrenheitToCelsius(maxTemperature);

        } else {
            console.log("Schedule temperature page: Schedule is undefined, so we can not save it.")
        }
    }
}
