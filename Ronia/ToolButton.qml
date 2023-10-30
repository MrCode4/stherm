import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

import Ronia

T.ToolButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 6
    spacing: 6

    icon.width: 24
    icon.height: 24
    icon.color: !enabled ? Style.hintTextColor : checked || highlighted ? Style.accent : Style.foreground

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display

        icon: control.icon
        text: control.text
        font: control.font
        color: !control.enabled ? Style.hintTextColor :
                control.checked || control.highlighted ? Style.accent : Style.foreground
    }

    background: Rectangle {
        implicitWidth: Style.button.buttonHeight
        implicitHeight: Style.button.buttonHeight
        height: width
        color: control.hovered ? (control.highlighted ? Style.highlightedRippleColor : Style.rippleColor)
                               : "transparent"
        radius: Math.min(width, height) / 2

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: control.pressed ? (control.highlighted ? Style.highlightedRippleColor : Style.rippleColor)
                                   : "transparent"
        }
    }
}
