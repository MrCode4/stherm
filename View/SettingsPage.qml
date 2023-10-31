import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SettingsPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

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
            if (deviceController) {
                //! Set setting using DeviceController
                deviceController.setSettings(_brightnessSlider.value,
                                             _speakerSlider.value,
                                             _tempFarenUnitBtn.checked ? AppSpec.TempratureUnit.Fah
                                                                       : AppSpec.TempratureUnit.Cel,
                                             _time24FormBtn.checked ? AppSpec.TimeFormat.Hour24
                                                                    : AppSpec.TimeFormat.Hour12,
                                             false, //! Reset
                                             _adaptiveBrSw.checked);
            }

            if (_root.StackView.view) {
                _root.StackView.view.pop();
            }
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
                value: appModel?.setting?.brightness ?? 0
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
                value: appModel?.setting?.volume ?? 0
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
                checked: appModel?.setting?.adaptiveBrightness ?? false
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
                checked: appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Fah
            }

            RadioButton {
                id: _tempCelciUnitBtn
                text: "\u00b0C"
                checked: appModel?.setting?.tempratureUnit === AppSpec.TempratureUnit.Cel
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
                checked: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour24
            }

            RadioButton {
                id: _time12FormBtn
                autoExclusive: false
                text: "12H"
                checked: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            }
        }
    }
}
