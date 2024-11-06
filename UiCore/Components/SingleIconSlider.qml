import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SingleIconSlider
 * ***********************************************************************************************/

RowLayout {
    id: root
    /* Property declaration
     * ****************************************************************************************/
    //! Labels postfix
    property string     labelSuffix:    ""

    //! Left and right colors to handle the slider color gradient.
    property string leftSideColor:  "#0097cd"
    property string rightSideColor: "#ea0600"

    property string icon: ""
    property real   iconSize: Qt.application.font.pointSize * 2

    property string title: ""

    //! Show the slider range
    property bool showRange: true

    //! Show ticks
    property bool showTicks: false

    //! Major tick count interval
    property int    majorTickCount: 3

    //! Total tick count to show
    property int    ticksCount: 15

    property int    scaleValue: 1

    property real from: 0
    property real to:   0

    //! To get the scaled control value
    readonly property real value: control.value / scaleValue

    property alias  control: _control

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        spacing: 4

        RoniaTextIcon {
            Layout.alignment: Qt.AlignHCenter

            font.pointSize: iconSize
            text: icon
            visible: icon.length > 0
            font.weight: FAIcons.Regular
        }

        Label {
            Layout.alignment: Qt.AlignHCenter

            text: title
            visible: text.length > 0
            font.pointSize: Qt.application.font.pointSize * 0.8

        }
    }

    Item {
        id: spacer
        height: parent.height
        width: 10
    }

    //! Lower range
    Label {
        opacity: 0.6
        font.pointSize: Qt.application.font.pointSize * 0.9
        text: _control.from.toLocaleString(locale, "f", Math.floor(scaleValue / 10)) + labelSuffix
        visible: showRange
    }

    Slider {
        id: _control

        Layout.fillWidth: true


        from: root.from * scaleValue
        to: root.to * scaleValue

        readonly property real tickStepSize: Math.abs(_control.to - _control.from) / ticksCount * scaleValue
        readonly property int majorTickInterval: majorTickCount * tickStepSize

        //! Value label
        Label {
            anchors.top: showTicks ? parent.top : parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 6
            anchors.topMargin: showTicks ? -30 : 6

            parent: _control.handle
            font.pointSize: Qt.application.font.pointSize * 0.9
            text: Number(root.value.toLocaleString(locale, "f", Math.floor(scaleValue / 10))) + labelSuffix
        }

        background: Rectangle {
            x: _control.leftPadding + (_control.horizontal ? 0 : (_control.availableWidth - width) / 2)
            y: _control.topPadding + (_control.horizontal ? (_control.availableHeight - height) / 2 : 0)
            implicitWidth: _control.horizontal ? 200 : 48
            implicitHeight: _control.horizontal ? 48 : 200
            width: _control.horizontal ? _control.availableWidth : 4
            height: _control.horizontal ? 4 : _control.availableHeight
            scale: _control.horizontal && _control.mirrored ? -1 : 1

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: _control.enabled ? leftSideColor : Qt.darker(leftSideColor, _control.darkerShade)
                }

                GradientStop {
                    position: 1.0
                    color: _control.enabled ? rightSideColor : Qt.darker(rightSideColor, _control.darkerShade)
                }
            }
        }

        //! Label ticks
        Item {
            id: ticksLabel

            width:  _control.availableWidth - _control.implicitHandleWidth
            height: implicitHeight
            x: _control.implicitHandleWidth / 2
            y: _control.implicitHandleHeight + 20
            visible: showTicks


            Repeater {
                model: Math.floor(ticksCount / majorTickCount) + 1
                delegate: Label {
                    readonly property int actualTickPos: index * majorTickCount
                    readonly property real calculatedValue:  Math.min(_control.to, _control.from) + index * majorTickCount
                    readonly property real textValue:  (calculatedValue > _control.to ? _control.to : calculatedValue)

                    y: 4
                    x: actualTickPos * (ticksLabel.width / ticksCount) - width / 2 * (scaleValue > 1 ? 1 : -1)
                    font.pointSize: Qt.application.font.pointSize * 0.75

                    scale: textValue == _control.value.toFixed(0) ? 1.0 : 0.8
                    Behavior on scale {
                        NumberAnimation {
                            duration: 250
                        }
                    }

                    opacity: 0.75
                    text: Number(textValue / scaleValue) + labelSuffix
                }
            }
        }
    }

    //! Upper range
    Label {
        opacity: 0.6
        font.pointSize: Qt.application.font.pointSize * 0.9
        text: _control.to.toLocaleString(locale, "f", Math.floor(scaleValue / 10)) + labelSuffix

        visible: showRange
    }
}
