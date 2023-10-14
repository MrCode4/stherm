import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Shapes

import Stherm

/*! ***********************************************************************************************
 * SemiCircleSlider
 * ***********************************************************************************************/
Slider {
    id: _control

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 400
    implicitHeight: 200
    leftInset: 0
    rightInset: 0
    background: Item {
        readonly property int pathWidth: 12
        readonly property real shapeWidth: shapeHeight * 2
        readonly property real shapeHeight: height - 20

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
    handle: Item {
        readonly property int angleRange: 180

        x: _control.width / 2
        y: _control.height - 10
        rotation: (value / Math.abs(to - from)) * angleRange

        Rectangle {
            x: -background.shapeWidth / 2 - width / 2 + 6
            y: - height / 2
            width: 20
            height: width
            radius: width / 2
        }
    }
}
