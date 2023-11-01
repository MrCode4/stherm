import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

import Ronia

T.ItemDelegate {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    padding: 16
    verticalPadding: 8
    spacing: 16

    icon.width: 24
    icon.height: 24
    icon.color: enabled ? Material.foreground : Material.hintTextColor

    contentItem: IconLabel {
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: control.display === IconLabel.IconOnly || control.display === IconLabel.TextUnderIcon ? Qt.AlignCenter : Qt.AlignLeft

        icon: control.icon
        text: control.text
        font: control.font
        color: control.enabled ? Style.foreground : Style.hintTextColor
    }

    background: Rectangle {
        implicitHeight: Style.delegateHeight

        color: control.pressed ? Style.rippleColor : (control.highlighted ? Style.listHighlightColor : "transparent")

        Rectangle {
            anchors.fill: parent
            color: control.hovered ? Style.rippleColor : "transparent"
        }
    }
}

