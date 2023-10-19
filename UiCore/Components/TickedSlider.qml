import QtQuick
import QtQuick.Controls.Material.impl
import QtQuick.Layouts

import Ronia

/*! ***********************************************************************************************
 * TickedSlider is an special slider with value ticks
 * ***********************************************************************************************/
Slider {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! Major tick count interval
    property int   majorTickCount: 3

    //! Total tick count to show
    property int   ticksCount: 15

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitHandleWidth + leftPadding + rightPadding,
                            ticksCount * (2 + 2) + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitHandleHeight + topPadding + bottomPadding)
                    + _ticksLay.implicitHeight * _ticksLay.visible
    leftPadding: 12
    rightPadding: 12
    snapMode: Slider.SnapAlways

    handle: SliderHandle {
        x: _control.leftPadding + (_control.horizontal ? _control.visualPosition * (_control.availableWidth - width): (_control.availableWidth - width) / 2)
        y: _control.topPadding + (_control.horizontal ? 6 : _control.visualPosition * (_control.availableHeight - height))
        value: _control.value
        handleHasFocus: _control.visualFocus
        handlePressed: _control.pressed
        handleHovered: _control.hovered
    }

    background: Rectangle {
        x: _control.leftPadding + (_control.horizontal ? 0 : (_control.availableWidth - width) / 2)
        y: _control.topPadding + (_control.horizontal ? height + 6 : 0)
        implicitWidth: _control.horizontal ? 200 : 40
        implicitHeight: _control.horizontal ? 40 : 200
        width: _control.horizontal ? _control.availableWidth : 4
        height: _control.horizontal ? 4 : _control.availableHeight
        scale: _control.horizontal && _control.mirrored ? -1 : 1
        color: _control.enabled ? Qt.alpha(_control.Material.accentColor, 0.33) : _control.Material.sliderDisabledColor

        Rectangle {
            x: _control.horizontal ? 0 : (parent.width - width) / 2
            y: _control.horizontal ? (parent.height - height) / 2 : _control.visualPosition * parent.height
            width: _control.horizontal ? _control.position * parent.width : 4
            height: _control.horizontal ? 4 : _control.position * parent.height

            color: _control.enabled ? _control.Material.accentColor : _control.Material.sliderDisabledColor
        }
    }

    GridLayout {
        id: _ticksLay

        readonly property int tickStepSize: Math.round(Math.abs(to - from) / ticksCount)
        readonly property int majorTickInterval: majorTickCount * tickStepSize
        readonly property int ticksWidth: 4

        parent: background
        visible: majorTickInterval > 0 && majorTickCount < ticksCount
        x: horizontal ? implicitHandleWidth / 2: 0
        y: horizontal ? implicitHandleHeight : 0
        width: horizontal ? _control.availableWidth - implicitHandleWidth: implicitWidth
        height: horizontal ? implicitHeight : parent.height + 4
        columns: horizontal ? 1 : 2
        rows: horizontal ? 2 : 1
        rowSpacing: 0
        columnSpacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 18

            Repeater {
                id: _ticksRepeater
                model: ticksCount + 1
                delegate: Rectangle {
                    readonly property bool isMajor: (index % majorTickCount) === 0

                    x: index * (_ticksLay.width / ticksCount) - width / 2
                    width: horizontal ? _ticksLay.ticksWidth : (isMajor ? 16 : 8)
                    height: horizontal ? (isMajor ? 16 : 8) : _ticksLay.ticksWidth
                    opacity: isMajor ? 0.8 : 0.5
                    radius: Math.min(width, height) / 2
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 16
            Repeater {
                model: Math.floor(ticksCount / majorTickCount) + 1
                delegate: Label {
                    opacity: 0.8
                    x: index * majorTickCount * (_ticksLay.width / ticksCount) - width / 2
                    text: index * _ticksLay.majorTickInterval
                }
            }
        }
    }
}
