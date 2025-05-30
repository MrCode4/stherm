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
    property real   value: 18.0

    //! Min value of slider
    property real   from: 18.0

    //! Max value of slider
    property real   to: 30.0

    readonly property real darkerShade: 3.8

    //! Holds whether slider is being dragged
    readonly property alias pressed: _handleDh.dragging

    property color leftColor: "#ea0600"
    property color rightColor: "#0097cd"

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 400
    implicitHeight: 200
    leftInset: 0
    rightInset: 0
    topInset: 0
    bottomInset: 0
    onFromChanged: value = Math.max(from, value)
    onToChanged: value = Math.min(to, value)
    background: Item {
        readonly property int pathWidth: 14 * scaleFactor
        readonly property real shapeWidth: shapeHeight * 2
        readonly property real shapeHeight: height - pathWidth * 2

        Rectangle {
            y: (parent.shapeHeight + 6)
            x: (parent.width - parent.shapeWidth) / 2
            width: parent.pathWidth
            height: width
            radius: width / 2
            color: _control.enabled ? leftColor : Qt.darker(leftColor, _control.darkerShade)
        }

        Rectangle {
            y: (parent.shapeHeight + 6)
            x: (parent.shapeWidth + (parent.width - parent.shapeWidth) / 2 - width)
            width: parent.pathWidth
            height: width
            radius: width / 2
            color: _control.enabled ? rightColor : Qt.darker(rightColor, _control.darkerShade)
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
                        position: 1
                        color: _control.enabled ? rightColor : Qt.darker(rightColor, _control.darkerShade)
                    }
                    GradientStop {
                        position: 0
                        color: _control.enabled ? leftColor : Qt.darker(leftColor, _control.darkerShade)
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
                fillColor: Style.background

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

        readonly property real angleRange: 180.0

        parent: _control.background
        x: parent.width / 2
        y: parent.height - (_control.height - _control.background.shapeHeight) / 2
        rotation: {
            var valueRange = Math.abs(to - from);
            return (((value - from) / (valueRange > 0 ? valueRange : 1)) * angleRange) % 360
        }

        Rectangle {
            id: _handleCircle
            x: -background.shapeWidth / 2 - width / 2 + background.pathWidth / 2
            y: -height / 2
            width: AppStyle.size / 24
            height: width
            radius: width / 2
            color: enabled ? _control.Material.foreground
                           : (Qt.darker(_control.Material.foreground, _control.darkerShade))
        }
    }

    Item {
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: parent.width + 24
        height: parent.height + 24

        DragHandler {
            id: _handleDh
            property bool dragging: false
            property real startAngle: -1

            readonly property real maxThresholdRadiusSq: Math.pow(_control.background.shapeHeight + 24, 2)
            readonly property real minThresholdRadiusSq: Math.pow(_control.background.shapeHeight
                                                                  - _control.background.pathWidth - 24, 2)
            readonly property point center: Qt.point((parent.width + parent.x) / 2, parent.height + parent.y * 2)

            grabPermissions: dragging ? PointerHandler.CanTakeOverFromAnything
                                      : PointerHandler.ApprovesTakeOverByAnything
            target: null //! To handle dragging arbitrarily
            dragThreshold: 2
            onGrabChanged: function(grabState, point){
                //! See enum QPointingDevice::GrabTransition
                if (grabState === 1) {
                    //! Get press distance to center of semi circle
                    var pressToCircle = Qt.vector2d(centroid.position.x - center.x,
                                                    centroid.position.y - center.y)
                    var lenSquared = Math.pow(pressToCircle.x, 2) + Math.pow(pressToCircle.y, 2);

                    if (lenSquared > minThresholdRadiusSq && lenSquared < maxThresholdRadiusSq) {
                        //! Press must be on the handle circle (with a threshold
                        var pressAngle = Math.atan2(pressToCircle.y, pressToCircle.x) * 57.2958 + 180;
                        if (pressAngle > 270) {
                            pressAngle = 360 - pressAngle;
                        }

                        if (Math.abs(pressAngle - _handle.rotation) < 14) {
                            startAngle = pressAngle;
                            dragging = true;
                        }
                    }
                } else if (grabState === 2 || grabState === 0x30 || grabState === 0x20) {
                    startAngle = -1;
                    dragging = false;
                }
            }

            onActiveTranslationChanged: if (dragging) updateValue()

            function updateValue()
            {
                var newPos = Qt.point(centroid.position.x, centroid.position.y);

                var angle = Math.atan2(center.y - newPos.y, center.x - newPos.x) * 57.2958;

                //! Allow angle a little more beyond 0 and 180 to make sure 'to' and 'from' are
                //! always easily reached. This won't effect on value since it's always clamped
                //! between 'to' and 'from'
                if (angle > -8 || angle < -172) {
                    //! Set value based on angle
                    angle = angle < -170 ? angle + 360 : angle;
                    var diffAngle = angle - startAngle;
                    var newValue = value + diffAngle / (_handle.angleRange) * Math.abs(to - from);

                    value = Math.max(from, Math.min(to, newValue));

                    startAngle = angle;
                }
            }
        }

        //! This MouseArea is added so touch/press events that won't start dragging in DragHandler
        //! can be passed to other Items like SwipeView or Flickable
        MouseArea {
            enabled: !_handleDh.dragging
            width: _handleDh.dragging ? 0 : parent.width
            height: _handleDh.dragging ? 0 : parent.height
            anchors.centerIn: parent
            propagateComposedEvents: true
        }
    }
}
