import QtQuick

import Ronia

/*! ***********************************************************************************************
 * ColorSlider is an slider for selecting a color
 * ***********************************************************************************************/
Slider {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! Current selected color
    readonly property alias currentColor: _internal.currentColor
    width: 700

    /* Object properties
     * ****************************************************************************************/
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
                GradientStop { position: _internal.hue0 ; color: "#ff0000" }
                GradientStop { position: _internal.hue1 ; color: "#ffff00" }
                GradientStop { position: _internal.hue2 ; color: "#00ff00" }
                GradientStop { position: _internal.hue3 ; color: "#00ffff" }
                GradientStop { position: _internal.hue4 ; color: "#0000ff" }
                GradientStop { position: _internal.hue5 ; color: "#ff00ff" }
                GradientStop { position: _internal.hue6 ; color: "#ff0000" }
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
        color: currentColor
        border.width: 2
        border.color: _control.Material.foreground
    }

    /* Children
     * ****************************************************************************************/
    QtObject {
        id: _internal

        property color currentColor: "#FF0000"

        readonly property real hueSpan: 0.16666
        //! Position in slider color based on hue circle
        readonly property real hue0:    0.00000 //! #000000 OR #ffffff
        readonly property real hue1:    0.16666 //! #ffff00
        readonly property real hue2:    0.33334 //! #00ff00
        readonly property real hue3:    0.50000 //! #00ffff
        readonly property real hue4:    0.66666 //! #0000ff
        readonly property real hue5:    0.83334 //! #ff00ff
        readonly property real hue6:    1.00000 //! #ff0000
    }

    onPositionChanged: function(event) {
        //! Calculate red channel
        var r = (position < _internal.hue1 || position > _internal.hue5
                 ? 1
                 : (position < _internal.hue2
                    ? (_internal.hue2 - position) / _internal.hueSpan
                    : (position > _internal.hue4
                       ? (position - _internal.hue4) / _internal.hueSpan
                       : 0)
                    )
                 );

        //! Calculate green channel
        var g = (position < _internal.hue1
                 ? position / _internal.hueSpan
                 : (position < _internal.hue3
                    ? 1
                    : (position < _internal.hue4
                       ? (_internal.hue4 - position) / _internal.hueSpan
                       : 0)
                    )
                 );

        //! Calculate blue channel
        var b = (position < _internal.hue2
                 ? 0
                 : (position < _internal.hue3
                    ? (position - _internal.hue2) / _internal.hueSpan
                    : (position > _internal.hue5)
                      ? (_internal.hue6 - position) / _internal.hueSpan
                      : 1
                    )
                 );

        _internal.currentColor = Qt.rgba(r, g, b, 1);
    }
}
