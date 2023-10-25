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
            }

            RadioButton {
                id: _tempCelciUnitBtn
                text: "\u00b0C"
            }

            //! Time Format
            Label {
                opacity: 0.6
                Layout.fillWidth: true
                text: "Time Format"
            }

            RadioButton {
                id: _time24FormBtn
                text: "24H"
            }

            RadioButton {
                id: _time12FormBtn
                text: "12H"
            }
        }
    }
}
