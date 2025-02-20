import QtQuick
import QtQuick.Controls.Material.impl

import Ronia

Item {
    id: root
    implicitWidth: initialSize
    implicitHeight: initialSize

    property real value: 0
    property bool handleHasFocus: false
    property bool handlePressed: false
    property bool handleHovered: false
    readonly property int initialSize: 18
    readonly property var control: parent

    Rectangle {
        id: handleRect
        width: parent.width
        height: parent.height
        radius: width / 2
        scale: root.handlePressed ? 1.5 : 1
        color: root.control.enabled ? root.control.Material.accentColor : root.control.Material.sliderDisabledColor

        Behavior on scale {
            NumberAnimation {
                duration: 250
            }
        }
    }

    //! Hovered Rectangle
    Rectangle {
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: initialSize * 2 + (root.handlePressed * 4)
        height: width
        visible: handleHasFocus || handleHovered
        color: Style.highlightedRippleColor
        radius: width / 2

        Behavior on width { NumberAnimation { duration : 250 } }
    }

    //! Hovered Rectangle
    Rectangle {
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: initialSize * 2 + (root.handlePressed * 4)
        height: width
        visible: handlePressed
        color: Style.highlightedRippleColor
        radius: width / 2

        Behavior on width { NumberAnimation { duration : 250 } }
    }
}
