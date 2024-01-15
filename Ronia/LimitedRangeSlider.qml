import QtQuick

import Ronia
import Ronia.impl

/*! ***********************************************************************************************
 * LimitedRangeSlider
 * ************************************************************************************************/
Control {
    id: control

    property int orientation: Qt.Horizontal
    property real difference: 0
    property real from: 0.
    property real to: 1.
    readonly property bool horizontal: orientation === Qt.Horizontal
    property alias first: handles.first
    property alias second: handles.second

    background: Rectangle {
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        implicitWidth: control.horizontal ? 200 : 48
        implicitHeight: control.horizontal ? 48 : 200
        width: control.horizontal ? control.availableWidth : 4
        height: control.horizontal ? 4 : control.availableHeight
        scale: control.horizontal && control.mirrored ? -1 : 1
        color: control.enabled ? Qt.alpha(Style.accent, 0.33) : control.Material.sliderDisabledColor

        Rectangle {
            x: control.horizontal ? control.first.position * parent.width : 0
            y: control.horizontal ? 0 : control.second.visualPosition * parent.height
            width: control.horizontal ? control.second.position * parent.width - control.first.position * parent.width : 4
            height: control.horizontal ? 4 : control.second.position * parent.height - control.first.position * parent.height

            color: control.enabled ? Style.accent : control.Material.sliderDisabledColor
        }
    }

    contentItem: Item {
        Repeater {
            model: ObjectModel {
                Component.onCompleted: {
                    append(first.handle);
                    append(second.handle)
                }
            }
        }
    }

    RangeSliderHandles {
        id: handles
        anchors.fill: parent
        horizontal: control.horizontal
        to: Math.max(control.to, control.from)
        from: Math.min(control.to, control.from)
        difference: control.difference
        handleColor: Style.accent
    }
}
