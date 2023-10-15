import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material


/*! ***********************************************************************************************
 * ColorSlider is an slider for selecting a color
 * ***********************************************************************************************/
Slider {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! Current selected color
    readonly property alias currentColor: _internal.currentColor

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
                GradientStop { position: 0.0 ; color: "#ff0000" }
                GradientStop { position: 0.2 ; color: "#ffff00" }
                GradientStop { position: 0.4 ; color: "#00ff00" }
                GradientStop { position: 0.6 ; color: "#00ffff" }
                GradientStop { position: 0.8 ; color: "#0000ff" }
                GradientStop { position: 1.0 ; color: "#ff00ff" }
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
    }

    onPositionChanged: function(event) {
        var r = (position < 0.2 ? 1 : (position < 0.4
                                       ? (0.4 - position) / 0.2
                                       : (position > 0.8 ? (position - 0.8) / 0.2
                                                         : 0)
                                       )
                 );
        var g = (position < 0.2 ? position / 0.2
                                : (position < 0.6 ? 1 : (position < 0.8
                                                         ? (0.8 - position) / 0.2
                                                         : 0)
                                   )
                 );
        var b = (position < 0.4 ? 0 : (position < 0.6
                                       ? (position - 0.4) / 0.2
                                       : 1
                                       )
                 );

        _internal.currentColor = Qt.rgba(r, g, b, 1);
    }
}
