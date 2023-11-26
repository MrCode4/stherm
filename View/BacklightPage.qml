import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm
import "./Delegates"
/*! ***********************************************************************************************
 * BacklightPage is for tweaking back light
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Ref to Backlight model
    property Backlight          backlight: appModel?.backlight ?? null

    //! Saturated color
    readonly property color     unshadedColor: Qt.hsva(_colorSlider.value, 1., _brSlider.value);

    //! Selected backlight color from shade buttons
    readonly property color     selectedColor: {
        var clr = _shadeButtonsGrp.checkedButton?.shadeColor ?? Style.background;
        return Qt.hsva(clr.hsvHue, clr.hsvSaturation, _brSlider.value);
    }

    //! Whether shade buttons should be shown
    property bool               hasShades: true

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
            checked: backlight ? backlight.on : true
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
            onMoved: dummyShadeDelegate.checked = true
        }

        Label {
            Layout.topMargin: AppStyle.size / 48
            Layout.leftMargin: AppStyle.size / 120
            text: "Brightness"
        }

        //! Brightness slider
        BrightnessSlider {
            id: _brSlider
            Layout.fillWidth: true
            opacity: enabled ? 1. : 0.4
            onValueChanged: onlineTimer.startTimer()

            Component.onCompleted: {
                handle.color = Qt.binding(function() {
                            var clr = unshadedColor;
                            return Qt.rgba(clr.r * brightness, clr.g * brightness, clr.b * brightness, 1.);
                        });
            }
        }

        Label {
            Layout.topMargin: AppStyle.size / 48
            Layout.leftMargin: AppStyle.size / 120
            visible: hasShades
            text: "Shades Of White"
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

            readonly property color shadesColor: "#FF8200"
            readonly property int cellSize: 82 * scaleFactor
            readonly property int spacing: 4

            Layout.preferredWidth: _shadeButtonsRepeater.count * (cellSize + spacing)
            Layout.preferredHeight: cellSize
            Layout.alignment: Qt.AlignCenter
            opacity: enabled ? 1. : 0.4
            visible: hasShades

            Repeater {
                id: _shadeButtonsRepeater
                model: 5
                delegate: ShadeButtonDelegate {
                    required property int index
                    required property var modelData

                    x: index * (_buttonsRow.cellSize + _buttonsRow.spacing) + (cellSize - width) / 2
                    hoverEnabled: enabled
                    cellSize: _buttonsRow.cellSize

                    hue: _buttonsRow.shadesColor.hsvHue
                    saturation: index / 4.
                    value: _buttonsRow.shadesColor.hsvValue
                }
            }

            ShadeButtonDelegate {
                id: dummyShadeDelegate

                property int index: 5

                visible: false
                x: index * (_buttonsRow.cellSize + _buttonsRow.spacing) + (cellSize - width) / 2
                checked: false
                hoverEnabled: enabled
                cellSize: _buttonsRow.cellSize

                hue: unshadedColor.hsvHue
                saturation: 1
                value: unshadedColor.hsvValue
            }
        }
    }

    function setCurrentColor(color, shadeIndex)
    {
        var h = color.hsvHue;
        var s = color.hsvSaturation;
        var v = color.hsvValue;

        _brSlider.value = v;

        if (shadeIndex === dummyShadeDelegate.index) {
            //! Restore color to hue slider
            _colorSlider.value = h;
            dummyShadeDelegate.checked = true;
        } else {
            _shadeButtonsRepeater.itemAt(shadeIndex).checked = true;
        }
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
            deviceController.updateBacklight(_backlightOnOffSw.checked, selectedColor,
                                             (_shadeButtonsGrp.checkedButton?.index ?? 0));
        }
    }

    //! reset backlight to model on cancel
    function revertToModel() {
        if (deviceController) {
            deviceController.updateBacklight(backlight?.on ?? false, backlight?._color ?? selectedColor,
                                             backlight?.shadeIndex ?? dummyShadeDelegate.index);
        }
    }

    Component.onCompleted: {
        if (backlight) {
            setCurrentColor(backlight._color, backlight.shadeIndex);
        }
        completed = true;
    }
}
