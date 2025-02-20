import QtQuick
import QtQuick.Templates as T

import Ronia
import Ronia.impl

T.Button {
    id: control

    property int        radius:          Style.button.radius < 0 ? Math.min(width, height) / 2 : Style.button.radius
    property color      backgroundColor: !control.enabled ? Style.button.disabledColor
                                                          : control.flat ? Qt.color("transparent")
                                                                         : (control.checked ? Style.foreground :
                                                                                              control.highlighted ? (control.checked ? Style.foreground :
                                                                                                                                       Style.accent)
                                                                                                                  : Style.button.background
                                                                            )

    property color topBackgroundColor: control.enabled && (control.hovered || control.pressed)
                                       ? (control.checked ? Qt.darker(backgroundColor, 1.5) : Style.button.hoverColor)
                                       : Qt.color("transparent")

    property color textColor: !control.enabled ? Style.hintTextColor
                                               : (control.checked ? Style.background
                                                                  : control.highlighted ? Style.accent
                                                                                        : control.Material.foreground
                                                  )

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
        color: textColor
        spacing: control.spacing
        button: control
        checkBackground: Style.background
    }

    background: Rectangle {
        implicitWidth: Style.button.buttonHeight
        implicitHeight: Style.button.buttonHeight

        border.width: control.flat ? 0 : Style.button.borderWidth
        border.color: Style.frameColor
        radius: control.radius
        color: control.backgroundColor

        Rectangle {
            anchors.fill: parent
            anchors.margins: parent.border.width
            radius: parent.radius
            color: topBackgroundColor
        }
    }
}
