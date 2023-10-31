import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SettingsPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! UiPreferences
    property UiPreferences      uiPreferences: uiSession?.uiPreferences ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Settings"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Save settings
            if (uiPreferences && deviceController) {
                //! Set setting using DeviceController
                deviceController.setSettings(_brightnessSlider.value,
                                             _speakerSlider.value,
                                             _tempFarenUnitBtn.checked ? UiPreferences.TempratureUnit.Fah
                                                                       : UiPreferences.TempratureUnit.Cel,
                                             _time24FormBtn.checked ? UiPreferences.TimeFormat.Hour24
                                                                    : UiPreferences.TimeFormat.Hour12,
                                             false, //! Reset
                                             _adaptiveBrSw.checked);

                if (uiPreferences.brightness !== _brightnessSlider.value) {
                    uiPreferences.brightness = _brightnessSlider.value;
                }

                if (uiPreferences.volume !== _speakerSlider.value) {
                    uiPreferences.volume = _speakerSlider.value;
                }

                if (uiPreferences.adaptiveBrightness !== _adaptiveBrSw.checked) {
                    uiPreferences.adaptiveBrightness = _adaptiveBrSw.checked;
                }

                var timeFormat = _time24FormBtn.checked ? UiPreferences.TimeFormat.Hour24
                                                        : UiPreferences.TimeFormat.Hour12;
                if (uiPreferences.timeFormat !== timeFormat) {
                    uiPreferences.timeFormat = timeFormat;
                }

                var tempUnit = _tempFarenUnitBtn.checked ? UiPreferences.TempratureUnit.Fah
                                                         : UiPreferences.TempratureUnit.Cel
                if (uiPreferences.tempratureUnit !== tempUnit) {
                    uiPreferences.tempratureUnit = tempUnit;
                }
            }

            if (_root.StackView.view) {
                _root.StackView.view.pop();
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.9
        height: Math.min(implicitHeight, parent.height)
        spacing: 8

        Label {
            opacity: 0.6
            text: "Brightness"
        }

        //! Brightness row
        RowLayout {
            spacing: 8

            RoniaTextIcon {
                Layout.rightMargin: 16
                text: "\ue0c9" //! brightness icon
            }

            Label {
                opacity: 0.6
                font.pointSize: _root.font.pointSize * 0.9
                text: _brightnessSlider.from.toLocaleString(locale, "f", 0)
            }

            Slider {
                id: _brightnessSlider
                Layout.fillWidth: true
                from: 0
                to: 100
                value: uiPreferences?.brightness ?? 0
            }

            Label {
                opacity: 0.6
                font.pointSize: _root.font.pointSize * 0.9
                text: _brightnessSlider.to.toLocaleString(locale, "f", 0)
            }
        }

        Label {
            opacity: 0.6
            Layout.topMargin: 12
            text: "Speaker"
        }

        //! Speaker row
        RowLayout {
            spacing: 8

            RoniaTextIcon {
                Layout.rightMargin: 16
                text: "\uf6a8" //! volume icon
            }

            Label {
                opacity: 0.6
                font.pointSize: _root.font.pointSize * 0.9
                text: _speakerSlider.from.toLocaleString(locale, "f", 0)
            }

            Slider {
                id: _speakerSlider
                Layout.fillWidth: true
                from: 0
                to: 100
                value: uiPreferences?.volume ?? 0
            }

            Label {
                opacity: 0.6
                font.pointSize: _root.font.pointSize * 0.9
                text: _speakerSlider.to.toLocaleString(locale, "f", 0)
            }
        }

        //! Adaptive brightness
        RowLayout {
            Layout.topMargin: 12

            Label {
                opacity: 0.6
                Layout.fillWidth: true
                text: "Adaptive Brightness"
            }

            Switch {
                id: _adaptiveBrSw
                checked: uiPreferences?.adaptiveBrightness ?? false
            }
        }

        //! Temprature unit
        GridLayout {
            Layout.topMargin: 12
            columnSpacing: 8
            rowSpacing: 8
            columns: 3

            Label {
                opacity: 0.6
                Layout.fillWidth: true
                text: "Temprature"
            }

            RadioButton {
                id: _tempFarenUnitBtn
                text: "\u00b0F"
                checked: uiPreferences?.tempratureUnit === UiPreferences.TempratureUnit.Fah
            }

            RadioButton {
                id: _tempCelciUnitBtn
                text: "\u00b0C"
                checked: uiPreferences?.tempratureUnit === UiPreferences.TempratureUnit.Cel
            }

            //! Time Format
            Label {
                opacity: 0.6
                Layout.fillWidth: true
                text: "Time Format"
            }

            //! Use explicit ButtonGroup to avoid time format RadioButtons being mutually exclusive
            //! with temprature RadioButtons.
            ButtonGroup {
                id: _timeButtonGrp
                buttons: [_time24FormBtn, _time12FormBtn]
            }

            RadioButton {
                id: _time24FormBtn
                autoExclusive: false
                text: "24H"
                checked: uiPreferences?.timeFormat === UiPreferences.TimeFormat.Hour24
            }

            RadioButton {
                id: _time12FormBtn
                autoExclusive: false
                text: "12H"
                checked: uiPreferences?.timeFormat === UiPreferences.TimeFormat.Hour12
            }
        }
    }
}
