import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ShadeButtonDelegate is a delegate for selecting shades of a color
 * ***********************************************************************************************/
RoundButton {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Size of the place where this button should be placed
    property int            cellSize

    //! Color value
    property real           value

    //! Color hue
    property real           hue

    //! Saturation
    property real           saturation

    //! Actual color of shaded
    readonly property alias shadeColor: _shadeColor.color

    /* Object properties
     * ****************************************************************************************/
    y: enabled && checked ? -cellSize / 3.2 : 0
    height: width
    checkable: true
    flat: !checked
    down: checked
    padding: 20
    background: Rectangle {
        implicitWidth: cellSize * (enabled && checked ? 1 : 0.85)
        implicitHeight: background.implicitWidth
        radius: _root.radius
        color: !_root.enabled ? _root.Material.buttonDisabledColor
            : _root.checked || _root.highlighted ? _root.Material.accentColor : _root.Material.buttonColor


        Rectangle {
            width: parent.width
            height: parent.height
            radius: _root.radius
            visible: _root.hovered
            color: _root.Material.rippleColor
        }
    }

    /* Children
     * ****************************************************************************************/
    Rectangle {
        id: _shadeColor
        anchors.centerIn: parent
        width: parent.availableWidth
        height: width
        radius: width / 2
        border.width: 2
        border.color: _root.Material.foreground
        color: Qt.hsva(hue, saturation, value)
    }

    Behavior on implicitWidth { NumberAnimation { duration: 200 } }
    Behavior on y { NumberAnimation { duration: 200 } }
}
