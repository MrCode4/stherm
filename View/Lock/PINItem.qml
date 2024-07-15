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

    property string errorText: ""

    //! Binding broke when pin changed.
    property bool   isPINWrong: false


    property var          _pinTextFieldItemsms: Object.values(pinLayout.children).filter(item => item instanceof PINTextField)

    property PINTextField _focusedItem: null


    //! Send pin to parent
    signal sendPIN(pin: string)

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

                    // focus: true
                    showPin: root.showPin
                    focus: false
                    isPINWrong: root.isPINWrong

                    onFocusChanged: {
                        if (focus) {
                            root._focusedItem = pinField
                        }
                    }

                    onTextChanged: {
                        root.isPINWrong = false;

                        // Update focus to the next field if text length is 1
                        if (text.length > 0) {
                            root.nextField(pinField).forceActiveFocus();
                        } else {
                            // Move focus back if text is cleared
                            // root.previousField(pinField).forceActiveFocus();
                        }
                    }
                }
            }

            //! Show/hide the pin
            Item {
                Layout.alignment: Qt.AlignVCenter
                width: 30
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
            id: warnInfo

            Layout.alignment: Qt.AlignHCenter

            font.pointSize: root.font.pointSize * 0.8
            text: errorText
            elide: Text.ElideMiddle
            visible: isPINWrong
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
                    backgroundColor: enabled ? (hovered ? Style.foreground : Style.button.disabledColor) : "transparent"
                    topBackgroundColor: backgroundColor
                    textColor: enabled ? (hovered ? Style.background : Style.foreground) : "transparent"

                    width: 55
                    height: width

                    onClicked: {
                        if (_focusedItem)
                            _focusedItem.text = modelData;
                    }
                }
            }

            //! Use item to have wide tap area.
            Item {
                width: 55
                height: width
                RoniaTextIcon {
                    anchors.centerIn: parent
                    text: AppStyle.generalIcons.deleteLeft
                    font.pointSize: root.font.pointSize * 1.1
                }

                TapHandler {
                    onTapped: {
                        _focusedItem.text = "";
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

            enabled: !isPINWrong && _pinTextFieldItems.filter(item => item.text.length > 0).length === pinLength
            text: isLock ? "Lock" : "Unlock"
            frameColor: "transparent"

            onClicked: {
                var pin = "";
                for (var i = 0; i < _pinTextFieldItems.length; i++) {
                    pin += _pinTextFieldItems[i].text;
                }

                console.log("PIN ", pin)
                sendPIN(pin);
            }
        }
    }

    /* Functions
     * ****************************************************************************************/

    // Helper functions for managing focus
      function nextField(pinField) {
        var index = _pinTextFieldItems.indexOf(pinField)
          console.log("indexindex", index)
        return _pinTextFieldItems[(index + 1) % _pinTextFieldItems.length]
      }

      function previousField(pinField) {
        var index = _pinTextFieldItems.indexOf(pinField)
        return _pinTextFieldItems[(index - 1 + _pinTextFieldItems.length) % _pinTextFieldItems.length]
      }
}
