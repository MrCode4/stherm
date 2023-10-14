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
    value: 0
    background: Item {
        readonly property int pathWidth: 12
        readonly property real shapeWidth: width - 8
        readonly property real shapeHeight: height - 8

        layer.enabled: true
        layer.samples: 8

        Rectangle {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: (1. / 9.) * parent.height + height / 2 + 4
            anchors.leftMargin: 7
            width: 12//parent.pathWidth
            height: width
            radius: width / 2
            color: "#0097cd"
        }

        Rectangle {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: (1. / 9.) * parent.height + height / 2 + 4
            anchors.rightMargin: 7
            width: 12//parent.pathWidth
            height: width
            radius: width / 2
            color: "#ea0600"
        }

        Shape {
            anchors.fill: parent
            anchors.margins: 4

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
                    startAngle: 190
                    sweepAngle: 160
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
                    centerY: background.shapeHeight - 2
                    radiusX: background.shapeWidth / 2 - background.pathWidth
                    radiusY: background.shapeHeight  - background.pathWidth
                    startAngle: 190
                    sweepAngle: 160
                }
            }
        }
    }
    handle: Item {
        readonly property int angleRange: 160

        x: _control.width / 2
        y: _control.height - 2
        rotation: (value / Math.abs(to - from)) * angleRange + 10

        Rectangle {
            x: -background.shapeWidth / 2 - width / 2 + 6
            y: - height / 2
            width: 20
            height: width
            radius: width / 2
        }
    }
}
