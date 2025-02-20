import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

import Ronia

T.ToolButton {
    id: control

    property int touchMargin: 0

    property bool clickable: true

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 12
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
        property bool square: control.contentItem.width <= control.contentItem.height

        implicitWidth: Style.touchTarget + touchMargin
        implicitHeight: Style.touchTarget + touchMargin
        width: square ? control.height : control.width
        height: square ? control.height : control.height
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        color: control.hovered ? (control.checked || control.highlighted ? Style.highlightedRippleColor : Style.rippleColor)
                               : (control.checked || control.highlighted ? Style.highlightedRippleColor : "transparent")
        radius: Math.min(width, height) / 2

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: !control.clickable ? "transparent" :
                                        control.pressed ? (control.checked || control.highlighted ? Style.highlightedRippleColor : Style.rippleColor)
                                                        : (control.checked || control.highlighted ? Style.highlightedRippleColor : "transparent")
        }
    }
}
