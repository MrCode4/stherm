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
    property Backlight          backlight: appModel?.backlight ?? null

    //! Selected backlight color from shade buttons
    readonly property color     selectedColor: _shadeButtonsGrp.checkedButton?.shadeColor ?? Style.background

    /* Object properties
     * ****************************************************************************************/
    leftPadding: AppStyle.size / 12
    rightPadding: AppStyle.size / 12
    title: "Backlight"

    /* Children
     * ****************************************************************************************/
    RowLayout {
        parent: _root.header.contentItem
        spacing: 8

        //! Backlight on/off button
        Switch {
            id: _backlightOnOffSw
            checked: backlight?.on ?? false
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
                    deviceController.updateBacklight(_backlightOnOffSw.checked, selectedColor);
                }
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: _root.availableWidth
        spacing: AppStyle.size / 18
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
            Layout.topMargin: AppStyle.size / 48
            Layout.leftMargin: AppStyle.size / 120
            text: "Brightness"
        }

        //! Brightness slider
        BrightnessSlider {
            id: _brSlider
            Material.accent: _colorSlider.currentColor
            Layout.fillWidth: true
            opacity: enabled ? 1. : 0.4
        }

        Label {
            Layout.topMargin: AppStyle.size / 48
            Layout.leftMargin: AppStyle.size / 120
            text: "Shades"
        }

        //! Group for shade buttons
        ButtonGroup {
            id: _shadeButtonsGrp
            buttons: _buttonsRow.children
        }

        //! Shades of selected color
        Item {
            id: _buttonsRow

            readonly property int cellSize: AppStyle.size / 8

            Layout.preferredWidth: _shadeButtonsRepeater.count * (cellSize + 8)
            Layout.preferredHeight: cellSize
            Layout.alignment: Qt.AlignCenter
            opacity: enabled ? 1. : 0.4

            Repeater {
                id: _shadeButtonsRepeater
                model: 5
                delegate: ShadeButtonDelegate {
                    x: index * (_buttonsRow.cellSize + 8) + (cellSize - width) / 2
                    checked: index === 4
                    hoverEnabled: enabled
                    cellSize: _buttonsRow.cellSize

                    hue: _colorSlider.value
                    saturation: index / 4.
                    value: _brSlider.value
                }
            }
        }
    }

    function setCurrentColor(color)
    {
        var h = color.hsvHue;
        var s = color.hsvSaturation;
        var v = color.hsvValue;

        _colorSlider.value = h;
        _brSlider.value = v;
        var shadeToSelect = Math.max(0, Math.ceil(s / 0.2) - 1);
        _shadeButtonsRepeater.itemAt(shadeToSelect).checked = true;
    }

    Component.onCompleted: {
        if (backlight) {
            setCurrentColor(backlight._color);
        }
    }
}
