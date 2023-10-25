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
        parent: _root.header
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Save settings
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.85
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
                value: uiPreferences?.brightness

                onValueChanged: {
                    if (uiPreferences && uiPreferences.brightness !== value) {
                        uiPreferences.brightness = value;
                    }
                }
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
                value: uiPreferences?.volume

                onValueChanged: {
                    if (uiPreferences && uiPreferences.volume !== value) {
                        uiPreferences.volume = value;
                    }
                }
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
                checked: uiPreferences?.adaptiveBrightness

                onToggled: {
                    if (uiPreferences && uiPreferences.adaptiveBrightness !== checked) {
                        uiPreferences.adaptiveBrightness = checked;
                    }
                }
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

                onToggled: {
                    if (uiPreferences && uiPreferences.tempratureUnit !== UiPreferences.TempratureUnit.Fah) {
                        uiPreferences.tempratureUnit = UiPreferences.TempratureUnit.Fah;
                    }
                }
            }

            RadioButton {
                id: _tempCelciUnitBtn
                text: "\u00b0C"
                checked: uiPreferences?.tempratureUnit === UiPreferences.TempratureUnit.Cel

                onToggled: {
                    if (uiPreferences && uiPreferences.tempratureUnit !== UiPreferences.TempratureUnit.Cel) {
                        uiPreferences.tempratureUnit = UiPreferences.TempratureUnit.Cel;
                    }
                }
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

                onToggled: {
                    if (uiPreferences && uiPreferences.timeFormat !== UiPreferences.TimeFormat.Hour24) {
                        uiPreferences.timeFormat = UiPreferences.TimeFormat.Hour24;
                    }
                }
            }

            RadioButton {
                id: _time12FormBtn
                autoExclusive: false
                text: "12H"
                checked: uiPreferences?.timeFormat === UiPreferences.TimeFormat.Hour12

                onToggled: {
                    if (uiPreferences && uiPreferences.timeFormat !== UiPreferences.TimeFormat.Hour12) {
                        uiPreferences.timeFormat = UiPreferences.TimeFormat.Hour12;
                    }
                }
            }
        }
    }
}
