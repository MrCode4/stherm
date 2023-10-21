import QtQuick
import QtQuick.Shapes

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SemiCircleSlider
 * ***********************************************************************************************/
Control {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! Value of Slider
    //! \NOTE: Any bindings to this value will be broken, use Connections instead
    property real   value: 0

    //! Min value of slider
    property real   from: 0

    //! Max value of slider
    property real   to: 10

    //! Holds whether slider is being dragged
    readonly property alias pressed: _handleMa.dragging

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size / 1.2
    implicitHeight: AppStyle.size / 2.4
    leftInset: 0
    rightInset: 0
    background: Item {
        readonly property int pathWidth: AppStyle.size / 40
        readonly property real shapeWidth: shapeHeight * 2
        readonly property real shapeHeight: height - AppStyle.size / 24

        layer.enabled: true
        layer.samples: 8

        Rectangle {
            y: (parent.shapeHeight + 4)
            x: (parent.width - parent.shapeWidth) / 2
            width: parent.pathWidth
            height: width
            radius: width / 2
            color: "#0097cd"
        }

        Rectangle {
            y: (parent.shapeHeight + 4)
            x: (parent.shapeWidth + (parent.width - parent.shapeWidth) / 2 - width)
            width: parent.pathWidth
            height: width
            radius: width / 2
            color: "#ea0600"
        }

        Shape {
            anchors.centerIn: parent
            height: parent.shapeHeight
            width: parent.shapeWidth

            ShapePath {
                startX: 0
                startY: background.shapeHeight
                capStyle: ShapePath.RoundCap
                strokeColor: "transparent"
                fillGradient: LinearGradient {
                    x1: 0
                    y1: 0
                    x2: background.shapeWidth
                    y2: y1

                    GradientStop {
                        position: 0
                        color: "#0097cd"
                    }
                    GradientStop {
                        position: 1
                        color: "#ea0600"
                    }
                }

                PathAngleArc {
                    centerX: background.shapeWidth / 2
                    centerY: background.shapeHeight
                    radiusX: background.shapeWidth / 2
                    radiusY: background.shapeHeight
                    startAngle: 180
                    sweepAngle: 180
                }
            }

            ShapePath {
                startX: 10
                startY: background.shapeHeight
                capStyle: ShapePath.RoundCap
                strokeColor: "transparent"
                fillColor: _control.Material.background

                PathAngleArc {
                    centerX: background.shapeWidth / 2
                    centerY: background.shapeHeight
                    radiusX: background.shapeWidth / 2 - background.pathWidth
                    radiusY: background.shapeHeight  - background.pathWidth
                    startAngle: 180
                    sweepAngle: 180
                }
            }
        }
    }

    Item {
        id: _handle
        readonly property int angleRange: 180

        x: _control.width / 2
        y: _control.height - AppStyle.size / 48
        rotation: {
            var valueRange = Math.abs(to - from);
            return (value / (valueRange > 0 ? valueRange : 1)) * angleRange
        }

        Rectangle {
            x: -background.shapeWidth / 2 - width / 2 + AppStyle.size / 80
            y: - height / 2
            width: AppStyle.size / 24
            height: width
            radius: width / 2

            MouseArea {
                id: _handleMa

                property bool dragging: false

                anchors.fill: parent
                anchors.margins: -8 //! To increase its size
                enabled: Math.abs(_control.to - _control.from) > 0
                preventStealing: true

                onPressed: dragging = true
                onReleased: dragging = false
                onCanceled: dragging = false
                onPositionChanged: function(event) {
                    //! Get angle to center
                    var center = Qt.point(_control.background.shapeWidth / 2, _control.height);
                    var point = mapToItem(_control, event.x, event.y);

                    //! Angle in degree
                    var angle = Math.atan2(center.y - point.y, center.x - point.x) * 57.2958;

                    //! If angle < 0 don't update value
                    if (angle >= 0) {
                        //! Set value based on angle
                        value = Math.min(to, Math.max(from, angle / _handle.angleRange) * Math.abs(to - from));
                    }
                }
            }
        }
    }
}
