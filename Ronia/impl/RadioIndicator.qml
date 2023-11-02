import QtQuick
import QtQuick.Templates as T

import Ronia

Rectangle {
    id: indicator

    implicitWidth: Style.radioButton.indicatorSize
    implicitHeight: Style.radioButton.indicatorSize
    radius: width / 2
    border.width: 2
    border.color: !control.enabled ? control.Material.hintTextColor
        : control.checked || control.down ? control.Material.accentColor : control.Material.secondaryTextColor
    color: "transparent"

    property T.AbstractButton control

    Rectangle {
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Style.radioButton.indicatorInnerCircleSize
        height: Style.radioButton.indicatorInnerCircleSize
        radius: width / 2
        color: parent.border.color
        visible: indicator.control.checked || indicator.control.down
    }
}
