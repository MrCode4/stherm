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

    //! The final color to apply to model and device
    readonly property color     liveColor: backlight?.backlightFinalColor(_shadeButtonsGrp.checkedButton?.index ?? dummyShadeDelegate.index,
                                                                          _colorSlider.value,
                                                                          _brSlider.value)

    //!
    property bool               completed: false

    //! Used when backlight changed in test mode
    property bool               isTest: false

    //!
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
    title: "Backlight"

    backButtonCallback: function() {
        //! Check if color is modified
        if (backlight && (backlight.on !== _backlightOnOffSw.checked
                          || !Qt.colorEqual(backlight._color, liveColor))) {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(goBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);
        } else {
            goBack();
        }
    }

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
            id: confirmtBtn
            Layout.alignment: Qt.AlignCenter
            contentItem: RoniaTextIcon {
                text: "\uf00c"
            }

            onClicked: {
                applyToModel();

                if (!isTest)
                    goBack();
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: _root.availableWidth
        spacing: 24
        opacity: _backlightOnOffSw.checked ? 1. : 0.4

        Label {
            Layout.leftMargin: 8 * scaleFactor
            enabled: _backlightOnOffSw.checked
            text: "Color"
        }

        //! Color slider
        ColorSlider {
            id: _colorSlider
            Layout.fillWidth: true
            onValueChanged: onlineTimer.startTimer()
            onMoved: dummyShadeDelegate.checked = true
            onPressedChanged: {
                if (pressed && !_backlightOnOffSw.checked) { {
                        _backlightOnOffSw.toggle();
                    }
                }
            }
        }

        Label {
            Layout.topMargin: AppStyle.size / 48
            Layout.leftMargin: 8 * scaleFactor
            text: "Brightness"
        }

        //! Brightness slider
        BrightnessSlider {
            id: _brSlider
            Layout.fillWidth: true
            opacity: enabled ? 1. : 0.4
            from: 0
            to: 1.
            onValueChanged: onlineTimer.startTimer()

            onPressedChanged: {
                if (pressed && !_backlightOnOffSw.checked) { {
                        _backlightOnOffSw.toggle();
                    }
                }
            }

            Component.onCompleted: {
                handle.color = Style.background;
            }
        }

        Label {
            Layout.topMargin: AppStyle.size / 48
            Layout.leftMargin: 8 * scaleFactor
            visible: true
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

            readonly property int cellSize: 80
            readonly property int spacing: 4

            Layout.preferredWidth: _shadeButtonsRepeater.count * (cellSize + spacing)
            Layout.preferredHeight: cellSize
            Layout.alignment: Qt.AlignCenter
            opacity: enabled ? 1. : 0.4
            visible: true

            Repeater {
                id: _shadeButtonsRepeater
                model: 5
                delegate: ShadeButtonDelegate {
                    required property int index
                    required property var modelData

                    x: index * (_buttonsRow.cellSize + _buttonsRow.spacing) + (cellSize - width) / 2
                    hoverEnabled: enabled
                    cellSize: _buttonsRow.cellSize

                    hue: backlight._whiteShade.hsvHue
                    saturation: index / 4.
                    value: backlight._whiteShade.hsvValue

                    onPressedChanged: {
                        if (pressed && !_backlightOnOffSw.checked) { {
                                _backlightOnOffSw.toggle();
                            }
                        }
                    }
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

                hue: _colorSlider.value
                saturation: 1
                value: _brSlider.value

                onPressedChanged: {
                    if (pressed && !_backlightOnOffSw.checked) { {
                            _backlightOnOffSw.toggle();
                        }
                    }
                }
            }
        }
    }

    function setCurrentColor(shadeIndex)
    {
        _brSlider.value = backlight.value;
        _colorSlider.value = backlight.hue;

        if (shadeIndex === dummyShadeDelegate.index) {
            //! Restore color to hue slider
            dummyShadeDelegate.checked = true;
        } else {
            _shadeButtonsRepeater.itemAt(shadeIndex).checked = true;
        }
    }

    //! Update backlight for test
    function applyOnline(){
        if (deviceController && completed) {
            deviceController.updateDeviceBacklight(_backlightOnOffSw.checked, liveColor);
        }
    }

    //! Update backlight and set to model
    function applyToModel() {
        if (deviceController) {
            deviceController.updateBacklight(_backlightOnOffSw.checked, _colorSlider.value, _brSlider.value,
                                             (_shadeButtonsGrp.checkedButton?.index ?? dummyShadeDelegate.index), isTest);
        }
    }

    //! reset backlight to model on cancel
    function revertToModel() {
        if (deviceController) {
            deviceController.updateBacklight(backlight?.on ?? false, backlight?.hue ?? 0., backlight?.value ?? 1.,
                                             backlight?.shadeIndex ?? dummyShadeDelegate.index);
        }
    }

    //! This method is used to go back
    function goBack()
    {
        uiSession.popUps.exitConfirmPopup.accepted.disconnect(confirmtBtn.clicked);
        uiSession.popUps.exitConfirmPopup.rejected.disconnect(goBack);

        if (_root.StackView.view) {
            //! Then Page is inside an StackView
            if (_root.StackView.view.currentItem == _root) {
                _root.StackView.view.pop();
            }
        }

        // Revert when change backlight with go back in test mode
        if (isTest) {
            revertToModel();
        }
    }

    Component.onCompleted: {
        if (backlight) {
            setCurrentColor(backlight.shadeIndex);
        }
        completed = true;
    }

    Component.onDestruction: {
        //! Revert to the color in model if last liveColor is not confirmed
        if (!Qt.colorEqual(backlight._color, liveColor)) {
            revertToModel();
        }
    }
}
