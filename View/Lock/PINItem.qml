import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A delegate to be used in Lock/Unlock page
 * ***********************************************************************************************/

Control {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property int pinLength: 4

    property bool showPin: false

    /* Object properties
     * ****************************************************************************************/


    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width
        spacing: 8

        RowLayout {
            spacing: 32
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                model: pinLength

                delegate: TextField {
                    id: delegate

                    readOnly: true
                    echoMode: showPin ? TextInput.Normal : TextInput.Password
                    validator: IntValidator {
                        bottom: 0
                        top: 9
                    }

                    background: Rectangle {
                        width: 35
                        height: 45
                        radius: 5
                        color: Style.button.disabledColor

                        border.width: delegate.text.length > 0 ? 2 : 0
                        border.color: Style.foreground
                    }
                }
            }

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

        GridLayout {
            columns: 4
            rows: 3


        }

    }
}
