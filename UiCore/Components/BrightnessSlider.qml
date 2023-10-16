import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material


/*! ***********************************************************************************************
 * BrightnessSlider is a slider for changint brightness of a color
 * ***********************************************************************************************/
Slider {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    readonly property real   brightness: visualPosition

    /* Object properties
     * ****************************************************************************************/
    value: Math.max(to, from)
    background: Item {
        x: _control.leftPadding + (_control.horizontal ? 0 : (_control.availableWidth - width) / 2)
        y: _control.topPadding + (_control.horizontal ? (_control.availableHeight - height) / 2 : 0)
        implicitWidth: _control.horizontal ? 200 : 32
        implicitHeight: _control.horizontal ? 32 : 200
        width: _control.availableWidth
        height: _control.availableHeight

        Rectangle {
            anchors.centerIn: parent
            width: horizontal? parent.width : 6
            height: horizontal? 6 : parent.height
            radius: Math.min(height, width) / 2
            gradient: Gradient {
                orientation: _control.orientation
                GradientStop { position: 0.0 ; color: "#000000" }
                GradientStop { position: 1.0 ; color: _control.Material.foreground}
            }
        }
    }
    handle: Rectangle {
        id: _handle
        x: _control.leftPadding + (_control.horizontal ? _control.visualPosition * (_control.availableWidth - width) : (_control.availableWidth - width) / 2)
        y: _control.topPadding + (_control.horizontal ? (_control.availableHeight - height) / 2 : _control.visualPosition * (_control.availableHeight - height))
        implicitWidth: 16
        implicitHeight: 16
        radius: width / 2
        color: {
            var clr = _control.Material.foreground;
            return Qt.rgba(clr.r * brightness, clr.g * brightness, clr.b * brightness, 1.);
        }
        border.width: 2
        border.color: _control.Material.foreground
    }
}
