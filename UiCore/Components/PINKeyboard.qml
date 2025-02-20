import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * PINItem: A delegate to be used in Lock/Unlock page
 * ***********************************************************************************************/

Control {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property bool   showPin: false

    property bool   isLock: true

    property int    pinLength: 4

    property string errorText: retryTimer.running ? `Next attempt available in ${_retryCountTime}` +
                                                    ((_retryCountTime > 1) ? ` secs.` : " sec.") :
                                                    isPinWrong ? `Wrong PIN, you have ${_retryCounter} more attempts.` : ""
    //! Binding broke when pin changed.
    property bool   isPinWrong: false


    property var          _pinTextFieldItems: Object.values(pinLayout.children).filter(item => item instanceof PINTextField)

    //! The PINTextField (PINs) that is currently receiving keyboard input
    property PINTextField _focusedItem: null

    //! Use to retry with new pin
    property int          _retryCounter: 3

    //! Can retry after _retryCountTime secs
    property int          _retryCountTime: 120


    //! Send pin to parent
    signal sendPIN(pin: string)
    signal clearPIN();
    signal forgetPIN();

    onClearPIN: {
        _pinTextFieldItems.forEach(item => {
                                      item.text = "";
                                   });

        if (_pinTextFieldItems.length > 0)
            _pinTextFieldItems[0].forceActiveFocus();
    }

    /* Object properties
     * ****************************************************************************************/

    Component.onCompleted: {
        if (_pinTextFieldItems.length > 0)
            _pinTextFieldItems[0].forceActiveFocus();
    }

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            id: pinLayout
            spacing: 8
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                id:pinRepeater

                model: pinLength

                PINTextField {
                    id: pinField

                    showPin: root.showPin
                    focus: false
                    isPinWrong: root.isPinWrong

                    onFocusChanged: {
                        if (focus) {
                            root._focusedItem = pinField
                        }
                    }

                    onTextChanged: {
                        // Update focus to the next field if text length is 1
                        if (text.length > 0) {
                            root.nextField(pinField).forceActiveFocus();
                        }
                    }
                }
            }

            //! Show/hide the pin
            Item {
                Layout.alignment: Qt.AlignVCenter
                width: 35
                height: 45

                RoniaTextIcon {
                    anchors.centerIn: parent
                    font.weight: 400
                    font.pointSize: root.font.pointSize * 0.8
                    text: showPin ? AppStyle.generalIcons.visible : AppStyle.generalIcons.hide
                }

                TapHandler{
                    onTapped: {
                        showPin = !showPin;
                    }
                }
            }
        }

        //! Show wrong PIN error.
        Label {
            id: errInfo

            Layout.alignment: Qt.AlignHCenter

            font.pointSize: root.font.pointSize * 0.8
            text: errorText
            elide: Text.ElideMiddle
            visible: isPinWrong
            color: Style.testFailColor
        }

        //! PIN keyboard
        GridLayout {
            columns: 4
            rows: 3
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight

                model: [
                    "1", "2", "3", "4",
                    "5", "6", "7", "8",
                    "", "9", "0"
                ]

                delegate: Button {
                    text: modelData

                    enabled: modelData.length > 0
                    flat: true
                    radius: 0
                    backgroundColor: enabled ? ((hovered || pressed) ? Style.foreground : Style.button.disabledColor) : "transparent"
                    topBackgroundColor: backgroundColor
                    textColor: enabled ? ((hovered || pressed) ? Style.background : Style.foreground) : "transparent"

                    width: 58
                    height: width

                    onClicked: {
                        if (_focusedItem) {
                            if (_focusedItem.text.length > 0) {
                                root.nextField(_focusedItem).forceActiveFocus();
                            }

                            if (_focusedItem)
                            _focusedItem.text = modelData;
                        }
                    }
                }
            }

            //! Use item to have wide tap area.
            Item {
                width: 58
                height: width

                Image {
                    anchors.centerIn: parent
                    width: 35
                    height: 21
                    source: "qrc:/Stherm/Images/delete-left.svg"
                    fillMode: Image.PreserveAspectFit
                    horizontalAlignment: Image.AlignHCenter
                    verticalAlignment: Image.AlignVCenter
                }
                TapHandler {
                    onTapped: {
                        _focusedItem.text = "";

                        // Move focus back if text is cleared
                        root.previousField(_focusedItem).forceActiveFocus();
                    }
                }
            }
        }

        ButtonInverted {
            Layout.alignment: Qt.AlignHCenter

            leftPadding: 8
            rightPadding: 8
            width: 100
            implicitWidth: width

            enabled: !retryTimer.running && _pinTextFieldItems.filter(item => item.text.length > 0).length === pinLength
            text: isLock ? "Lock" : "Unlock"
            frameColor: "transparent"

            onClicked: {
                // Reset the _retryCountTime
                _retryCountTime = 120;

                var pin = "";
                for (var i = 0; i < _pinTextFieldItems.length; i++) {
                    pin += _pinTextFieldItems[i].text;
                }

                sendPIN(pin);
            }
        }
    }

    Timer {
        id: retryTimer

        repeat: true
        interval: 1000
        running: isPinWrong && _retryCounter < 1 && _retryCountTime > 0

        onRunningChanged: {
            if(running)
                forgetPIN();
        }

        onTriggered: {
            _retryCountTime--;

            // reset retry counter
            if (_retryCountTime <= 0) {
                isPinWrong = false;
                _retryCounter = 3;
            }
        }
    }

    /* Functions
     * ****************************************************************************************/

    // Helper functions for managing focus
    function nextField(pinField) {
        var index = _pinTextFieldItems.indexOf(pinField);
        var focusIndex = (index + 1) % _pinTextFieldItems.length;
        if (index + 1 === _pinTextFieldItems.length)
            focusIndex = index;

        return _pinTextFieldItems[focusIndex];
    }

    function previousField(pinField) {
        var index = _pinTextFieldItems.indexOf(pinField);
        var focusIndex = (index - 1 + _pinTextFieldItems.length) % _pinTextFieldItems.length;
        if (index === 0)
            focusIndex = index;

        return _pinTextFieldItems[focusIndex];
    }

    //! Check PIN correctness
    function updatePinStatus(isCorrect : bool) {
        isPinWrong = !isCorrect
        if (isPinWrong) {
            _retryCounter--;

        } else {
            // reset retry counter
            _retryCounter = 3;
        }
    }
}
