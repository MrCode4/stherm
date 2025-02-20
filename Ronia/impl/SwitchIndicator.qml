import QtQuick
import QtQuick.Templates as T

import Ronia

Item {
    id: indicator
    implicitWidth: 68
    implicitHeight: 36

    property T.AbstractButton control
    property alias handle: handle

    Rectangle {
        id: track
        width: parent.width
        height: 36
        radius: height / 2
        y: parent.height / 2 - height / 2
        color: indicator.control.enabled
               ? (indicator.control.checked ? Style.switchStyle.switchCheckedTrackColor
                                            : Style.switchStyle.switchUncheckedTrackColor)
               : Style.switchStyle.switchDisabledTrackColor
    }

    Rectangle {
        id: handle
        property int margins: track.height > height ? (track.height - height) / 2 : 0
        x: Math.max(margins, Math.min(parent.width - width - margins,
                                      indicator.control.visualPosition * parent.width - (width / 2)))
        y: (parent.height - height) / 2
        width: 26
        height: width
        radius: width / 2
        color: indicator.control.enabled ? (indicator.control.checked
                                            ? Style.switchStyle.switchCheckedHandleColor
                                            : Style.switchStyle.switchUncheckedHandleColor)
                                         : Style.switchStyle.switchDisabledHandleColor

        Behavior on x {
            enabled: !indicator.control.pressed
            SmoothedAnimation {
                duration: 300
            }
        }
    }
}
