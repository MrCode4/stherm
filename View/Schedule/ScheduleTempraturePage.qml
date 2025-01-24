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

    //! Temperature user selected value: this is always in celsius
    readonly property real  minimumTemperature: {
        var temperatureValue = _tempSlider.first.value;
        if (isHeating) {
            temperatureValue = singleTemperatureSlider.control.value;
        }

        // as UI shows rounded! should be in sync
        temperatureValue = temperatureValue.toFixed(0);

        return (isCelcius ? temperatureValue : Utils.fahrenheitToCelsius(temperatureValue));
    }

    // user selected value
    readonly property real  maximumTemperature: {
        var temperatureValue = _tempSlider.second.value;
        if (isCooling) {
            temperatureValue = singleTemperatureSlider.control.value;
        }

        // as UI shows rounded! should be in sync
        temperatureValue = temperatureValue.toFixed(0);

        return (isCelcius ? temperatureValue : Utils.fahrenheitToCelsius(temperatureValue))
    }

    //! Minimum temprature
    property real               minTemperature: deviceController?.getMinValue(schedule?.systemMode, temperatureUnit) ?? 40

    //! Maximum temprature
    property real               maxTemperature: deviceController?.getMaxValue(schedule?.systemMode, temperatureUnit) ?? 90

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

                labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(temperatureUnit))
                leftLabelPrefix:  `<p style='color:${AppSpec.heatingColor}; font-size:12px;'>Heat to</p>`
                rightLabelPrefix: `<p style='color:${AppSpec.coolingColor}; font-size:12px;'>Cool to</p>`
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

                text: `<span style='color:${AppSpec.heatingColor};'>Heat to</span> - Heating will be turned off when the indoor temperature is above this value`
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                horizontalAlignment: Qt.AlignLeft
                font.pointSize: root.font.pointSize * 0.67
            }

            Label {
                Layout.preferredWidth: parent.width / 2 - 5

                text: `<span style='color:${AppSpec.coolingColor};'>Cool to</span> - Cooling will be turned off when the indoor temperature is below this value`
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
            leftSideColor:  {
                if (schedule && schedule.systemMode === AppSpec.Cooling) {
                        return AppSpec.coolingColor;
                }

                return AppSpec.heatingColor;
            }

            rightSideColor: {
                if (schedule && (schedule.systemMode === AppSpec.Heating || schedule.systemMode === AppSpec.EmergencyHeat)) {
                    return AppSpec.heatingColor;
                }

                return AppSpec.coolingColor;
            }

            labelSuffix: "\u00b0" + (AppSpec.temperatureUnitString(deviceController.temperatureUnit))
            from: minTemperature;
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
            let minTemperature = _tempSlider.first.value;
            let maxTemperature = _tempSlider.second.value;

            if (isHeating) {
                // Update minimum temperature as heating temperature
                minTemperature = singleTemperatureSlider.control.value;

            } else if (isCooling) {
                // Update maximum temperature as cooling temperature
                maxTemperature = singleTemperatureSlider.control.value;
            }

            // as UI shows rounded! should be in sync
            minTemperature = minTemperature.toFixed(0);
            maxTemperature = maxTemperature.toFixed(0);

            // Save temperatures as celcius.
            schedule.minimumTemperature = isCelcius ? minTemperature : Utils.fahrenheitToCelsius(minTemperature);
            schedule.maximumTemperature = isCelcius ? maxTemperature : Utils.fahrenheitToCelsius(maxTemperature);

        } else {
            console.log("Schedule temperature page: Schedule is undefined, so we can not save it.")
        }
    }
}
