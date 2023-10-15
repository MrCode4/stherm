import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm
import "../UiCore/Components"

/*! ***********************************************************************************************
 * BacklightPage is for tweaking back light
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 480
    implicitHeight: 480
    leftPadding: 40
    rightPadding: 40
    title: "Backlight Test"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent
        width: _root.availableWidth
        spacing: 16

        Label {
            Layout.leftMargin: 4
            text: "Color"
        }

        //! Color slider
        ColorSlider {
            id: _colorSlider
            Layout.fillWidth: true
        }

        Label {
            Layout.topMargin: 40
            Layout.leftMargin: 4
            text: "Brightness"
        }

        //! Brightness slider
        BrightnessSlider {
            id: _brSlider
            Material.foreground: _colorSlider.currentColor
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.topMargin: 32
            Layout.alignment: Qt.AlignCenter
            implicitWidth: 32
            implicitHeight: 32
            radius: width / 2
            color: {
                var clr = _colorSlider.currentColor;

                return Qt.rgba(clr.r * _brSlider.visualPosition,
                               clr.g * _brSlider.visualPosition,
                               clr.b * _brSlider.visualPosition,
                               1.,);
            }
        }
    }
}
