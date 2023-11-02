import QtQuick
import QtQuick.Templates as T

import Ronia
import Ronia.impl

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: Style.button.horizontalPadding
    verticalPadding: Style.button.verticalPadding
    spacing: 8
    topInset: 4
    bottomInset: 4

    icon.width: 24
    icon.height: 24
    icon.color: !enabled ? Style.hintTextColor : highlighted ? Style.accent : Material.foreground

    contentItem: ButtonLabel {
        font: control.font
        text: control.text
        color: !control.enabled ? Style.hintTextColor
                                : (control.checked ? Style.background
                                                   : control.highlighted ? Style.accent
                                                                         : control.Material.foreground
                                   )
        spacing: control.spacing
        button: control
        checkColor: control.checkable && control.checked ? Style.foreground : "transparent"
        checkBackground: Style.background
    }

    background: Rectangle {
        implicitWidth: Style.button.buttonHeight
        implicitHeight: Style.button.buttonHeight

        border.width: control.flat ? 0 : Style.button.borderWidth
        border.color: Style.frameColor
        radius: Style.button.radius < 0 ? Math.min(width, height) / 2 : Style.button.radius
        color: !control.enabled ? Style.button.disabledColor
                                : control.flat ? "transparent"
                                               : (control.checked ? Style.foreground :
                                                                    control.highlighted ? (control.checked ? Style.foreground :
                                                                                                             Style.accent)
                                                                                        : Style.button.background
                                                  )

        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            color: control.enabled && (control.hovered || control.pressed)
                   ? (control.checked ? Qt.darker(parent.color, 1.5) : Style.button.hoverColor)
                   : "transparent"
        }
    }
}
