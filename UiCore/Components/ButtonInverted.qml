import QtQuick

import Ronia
import Ronia.impl
import Stherm

/*! ***********************************************************************************************
 * ButtonInverted is a button that swap background and foreground color
 * ***********************************************************************************************/
Button {
    id: control

    property int radius: 8

    property color      frameColor: Style.frameColor

    property color textColor: !control.enabled ? Style.hintTextColor
                                               : Style.background

    contentItem: Label {
        font: control.font
        text: control.text
        color: textColor

        verticalAlignment: "AlignVCenter"
        horizontalAlignment: "AlignHCenter"
    }

    background: Rectangle {
        implicitWidth: 64
        implicitHeight: Style.button.buttonHeight

        border.width: control.flat ? 0 : Style.button.borderWidth
        border.color: frameColor
        radius: control.radius
        color: !control.enabled ? Style.button.disabledColor
                                : Style.foreground

        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            color: control.enabled && (control.hovered || control.pressed)
                   ? Qt.darker(parent.color, 1.2)
                   : "transparent"
        }
    }
}
