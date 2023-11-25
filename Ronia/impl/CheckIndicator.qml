import QtQuick

import Ronia

Rectangle {
    id: indicatorItem

    implicitWidth: 22
    implicitHeight: 22

    color: "transparent"
    border.color: !control.enabled ? Style.hintTextColor
                                   : checkState !== Qt.Unchecked ? Style.accent : Style.secondaryTextColor
    border.width: checkState !== Qt.Unchecked ? width / 2 : 2
    radius: 2

    property Item control
    property int checkState: control.checkState

    Behavior on border.width {
        NumberAnimation {
            duration: 100
            easing.type: Easing.OutCubic
        }
    }

    Behavior on border.color {
        ColorAnimation {
            duration: 100
            easing.type: Easing.OutCubic
        }
    }

    // TODO: This needs to be transparent
    Image {
        id: checkImage
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 16
        height: 16
        source: "qrc:/Ronia/impl/res/check-dark.png"
        fillMode: Image.PreserveAspectFit
        sourceSize.width: 16
        sourceSize.height: 16

        scale: indicatorItem.checkState === Qt.Checked ? 1 : 0
        Behavior on scale { NumberAnimation { duration: 100 } }
    }

    Rectangle {
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 12
        height: 3

        scale: indicatorItem.checkState === Qt.PartiallyChecked ? 1 : 0
        Behavior on scale { NumberAnimation { duration: 100 } }
    }

    states: [
        State {
            name: "checked"
            when: indicatorItem.checkState === Qt.Checked
        },
        State {
            name: "partiallychecked"
            when: indicatorItem.checkState === Qt.PartiallyChecked
        }
    ]

    transitions: Transition {
        SequentialAnimation {
            NumberAnimation {
                target: indicatorItem
                property: "scale"
                // Go down 2 pixels in size.
                to: 1 - 2 / indicatorItem.width
                duration: 120
            }
            NumberAnimation {
                target: indicatorItem
                property: "scale"
                to: 1
                duration: 120
            }
        }
    }
}
