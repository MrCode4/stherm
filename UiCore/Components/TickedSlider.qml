import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import QtQuick.Layouts

/*! ***********************************************************************************************
 * TickedSlider is an special slider with value ticks
 * ***********************************************************************************************/
Slider {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! Major tick value interval
    property int   majorTickCount: 10

    //! Step size of ticks
    property int    ticksCount: 2

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitHandleWidth + leftPadding + rightPadding)
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
        color: _control.enabled ? Color.transparent(_control.Material.accentColor, 0.33) : _control.Material.sliderDisabledColor

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

        parent: background
        visible: majorTickInterval > 0 && majorTickCount < ticksCount
        x: horizontal ? implicitHandleWidth / 2 - 3 : 0
        y: horizontal ? implicitHandleHeight : 0
        width: horizontal ? _control.availableWidth - _control.implicitHandleWidth + 6: implicitWidth
        height: horizontal ? implicitHeight : parent.height + 4
        columns: horizontal ? 1 : 2
        rows: horizontal ? 2 : 1
        rowSpacing: 0
        columnSpacing: 0

        GridLayout {
            Layout.preferredWidth: Math.max(parent.width, (ticksCount + 1) * 2)
            columns: horizontal ? 10000 : 1
            rows: horizontal ? 1 : 10000
            rowSpacing: 0
            columnSpacing: 0
            Repeater {
                id: _ticksRepeater
                model: ticksCount + 1
                delegate: Rectangle {
                    readonly property bool isMajor: (index % majorTickCount) === 0

                    Layout.alignment: Qt.AlignCenter
                    Layout.preferredWidth: horizontal ? 2 : (isMajor ? 16 : 8)
                    Layout.preferredHeight: horizontal ? (isMajor ? 16 : 8) : 2
                    opacity: isMajor ? 0.8 : 0.5
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 16
            Repeater {
                model: ((majorTickCount) * _ticksLay.majorTickInterval) <= Math.max(to, from)
                       ? majorTickCount + 1 : majorTickCount
                delegate: Label {
                    opacity: 0.8
                    x: index * majorTickCount * (_ticksLay.width / ticksCount) - width / 2
                    text: index * _ticksLay.majorTickInterval
                }
            }
        }
    }
}
