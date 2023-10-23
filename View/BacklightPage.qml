import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BacklightPage is for tweaking back light
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Ref to Backlight model
    property Backlight              backlight: uiSession?.appModel?.backlight ?? null

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
    leftPadding: AppStyle.size / 12
    rightPadding: AppStyle.size / 12
    title: "Backlight"

    /* Children
     * ****************************************************************************************/
    RowLayout {
        parent: _root.header
        spacing: 8

        //! Backlight on/off button
        Switch {
            id: _backlightOnOffSw
            checked: backlight?.on ?? false

            onToggled: {
                if (backlight && backlight.on !== checked) {
                    backlight.on = checked;
                }
            }
        }

        //! Confirm button
        ToolButton {
            Layout.alignment: Qt.AlignCenter
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
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: _root.availableWidth
        spacing: AppStyle.size / 30
        enabled: _backlightOnOffSw.checked

        Label {
            Layout.leftMargin: 4
            text: "Color"
        }

        //! Color slider
        ColorSlider {
            id: _colorSlider
            Layout.fillWidth: true
            opacity: enabled ? 1. : 0.4
        }

        Label {
            Layout.topMargin: AppStyle.size / 12
            Layout.leftMargin: AppStyle.size / 120
            text: "Brightness"
        }

        //! Brightness slider
        BrightnessSlider {
            id: _brSlider
            Material.foreground: _colorSlider.currentColor
            Layout.fillWidth: true
            opacity: enabled ? 1. : 0.4
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
            Layout.leftMargin: AppStyle.size / 30
            Layout.rightMargin: AppStyle.size / 30
            Layout.topMargin: AppStyle.size / 16
            opacity: enabled ? 1. : 0.4

            Repeater {
                id: _shadeButtonsRepeater
                model: 5
                delegate: ShadeButtonDelegate {
                    checked: index === 4
                    sourceColor: _slidersColorNotShaded
                    shadeFactor: index / 4.
                    hoverEnabled: enabled
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
