import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SingleTemperatureSlider
 * ***********************************************************************************************/

RowLayout {
    id: root
    /* Property declaration
     * ****************************************************************************************/
    //! Labels postfix
    property string     labelSuffix:    ""

    property alias      control: _control

    RoniaTextIcon {
        font.pointSize: Qt.application.font.pointSize * 2
        text: "\uf2c8" //! temperature-three-quarters icon
    }

    Item {
        id: spacer
        height: parent.height
        width: 10
    }

    Label {
        opacity: 0.6
        font.pointSize: Qt.application.font.pointSize * 0.9
        text: _control.from.toLocaleString(locale, "f", 0)
    }

    Slider {
        id: _control

        Layout.fillWidth: true

        //! Value label
        Label {
            anchors {
                top: parent.bottom
                horizontalCenter: parent.horizontalCenter
                margins: 6
            }
            parent: _control.handle
            font.pointSize: Qt.application.font.pointSize * 0.9
            text: Number(_control.value).toLocaleString(locale, "f", 0) + labelSuffix
        }

        background: Rectangle {
            x: _control.leftPadding + (_control.horizontal ? 0 : (_control.availableWidth - width) / 2)
            y: _control.topPadding + (_control.horizontal ? (_control.availableHeight - height) / 2 : 0)
            implicitWidth: _control.horizontal ? 200 : 48
            implicitHeight: _control.horizontal ? 48 : 200
            width: _control.horizontal ? _control.availableWidth : 4
            height: _control.horizontal ? 4 : _control.availableHeight
            scale: _control.horizontal && _control.mirrored ? -1 : 1

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: _control.enabled ? "#0097cd" : Qt.darker("#0097cd", _control.darkerShade)
                }

                GradientStop {
                    position: 1.0
                    color: _control.enabled ? "#ea0600" : Qt.darker("#ea0600", _control.darkerShade)
                }
            }
        }
    }

    Label {
        opacity: 0.6
        font.pointSize: Qt.application.font.pointSize * 0.9
        text: _control.to.toLocaleString(locale, "f", 0)
    }
}
