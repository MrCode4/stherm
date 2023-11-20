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

    property bool          completed: false

    property Timer onlineTimer: Timer {
        repeat: false
        running: false
        interval: 50
        onTriggered: applyOnline()
        function startTimer() {
            if (!onlineTimer.running) {
                start();
            }
        }
    }

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
            onCheckedChanged: onlineTimer.startTimer()
        }

        //! Confirm button
        ToolButton {
            Layout.alignment: Qt.AlignCenter
            contentItem: RoniaTextIcon {
                text: "\uf00c"
            }

            onClicked: applyToModel()
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
            onValueChanged: onlineTimer.startTimer()
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
            onValueChanged: onlineTimer.startTimer()
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

            onCheckedButtonChanged: onlineTimer.startTimer()
        }

        //! Shades of selected color
        Item {
            id: _buttonsRow

            readonly property int cellSize: 72 * scaleFactor
            readonly property int spacing: 4

            Layout.preferredWidth: _shadeButtonsRepeater.count * (cellSize + spacing)
            Layout.preferredHeight: cellSize
            Layout.alignment: Qt.AlignCenter
            opacity: enabled ? 1. : 0.4

            Repeater {
                id: _shadeButtonsRepeater
                model: 5
                delegate: ShadeButtonDelegate {
                    x: index * (_buttonsRow.cellSize + _buttonsRow.spacing) + (cellSize - width) / 2
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

    //! Update backlight for test
    function applyOnline(){
        if (deviceController && completed) {
            deviceController.updateDeviceBacklight(_backlightOnOffSw.checked, selectedColor);
        }
    }

    //! Update backlight and set to model
    function applyToModel() {
        if (deviceController) {
            deviceController.updateBacklight(_backlightOnOffSw.checked, selectedColor);
        }
    }

    //! reset backlight to model on cancel
    function revertToModel() {
        if (deviceController) {
            deviceController.updateBacklight(backlight?.on ?? false, backlight?._color ?? selectedColor);
        }
    }

    Component.onCompleted: {
        if (backlight) {
            setCurrentColor(backlight._color);
        }
        completed = true;
    }
}
