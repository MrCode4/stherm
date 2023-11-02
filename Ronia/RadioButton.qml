import QtQuick
import QtQuick.Templates as T

import Ronia
import Ronia.impl

T.RadioButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    spacing: 8
    padding: 8
    verticalPadding: padding + 6

    indicator: RadioIndicator {
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control

        Rectangle {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: parent.width + 16
            height: width

            z: -1
            radius: width / 2
            visible: control.hovered
            color: control.checked ? Style.highlightedRippleColor : Style.rippleColor
        }
    }

    contentItem: Text {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0

        text: control.text
        font: control.font
        color: control.enabled ? Style.foreground : Style.hintTextColor
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
