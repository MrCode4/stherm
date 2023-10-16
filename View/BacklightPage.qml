import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * BacklightPage is for tweaking back light
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Ref to Backlight model
    property Backlight              backlight: uiSession?.appModel?.backlight

    //! Selected backlight color from shade buttons
    readonly property color selectedColor: _shadeButtonsGrp.checkedButton?.shadeColor ?? Material.background

    //! Actual color calculated from sliders without shade
    readonly property color _slidersColorNotShaded: {
        var clr = _colorSlider.currentColor;
        return Qt.rgba(clr.r * _brSlider.visualPosition,
                       clr.g * _brSlider.visualPosition,
                       clr.b * _brSlider.visualPosition,
                       1.,)
    }

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 40
    rightPadding: 40
    title: "Backlight"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        Layout.alignment: Qt.AlignCenter
        parent: _root.header
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Update backlight
            if (deviceController) {
                deviceController.updateBacklight();
            }
        }
    }

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

        //! Group for shade buttons
        ButtonGroup {
            id: _shadeButtonsGrp
            buttons: _buttonsRow.children
        }

        //! Shades of selected color
        RowLayout {
            id: _buttonsRow
            Layout.preferredWidth: _root.availableWidth
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.topMargin: 32

            Repeater {
                id: _shadeButtonsRepeater
                model: 5
                delegate: ShadeButtonDelegate {
                    checked: index === 4
                    sourceColor: _slidersColorNotShaded
                    shadeFactor: index / 4.
                }
            }
        }
    }

    onSelectedColorChanged: {
        if (backlight) {
            backlight.color = selectedColor;
        }
    }
}
