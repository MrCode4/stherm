import QtQuick
import QtQuick.Shapes

import Ronia
import Ronia.impl 1.0
import Stherm

/*! ***********************************************************************************************
 * SemiCircleSliderDoubleHandle
 * ***********************************************************************************************/
Control {
    id: _control

    /* Property declaration
     * ****************************************************************************************/
    //! Min value of slider
    property real           from: 18.0

    //! Max value of slider
    property real           to: 30.0

    //! Difference between first and second values
    property real           difference: 2

    //! Show grey section
    property bool           showGreySection: true

    //! Difference between two values
    readonly property real  darkerShade: 3.8

    //! Holds whether slider is being dragged
    readonly property bool  pressed: first.pressed || second.pressed

    //! Some other limitations for first and second values
    property real firstValueCeil: AppSpec.maxAutoMinTemp
    property real secondValueFloor: AppSpec.minAutoMaxTemp

    onFirstValueCeilChanged: first.setMaxValue(firstValueCeil);
    onSecondValueFloorChanged: second.setMinValue(secondValueFloor);

    //! First handle data
    property RangeSliderHandleData first: RangeSliderHandleData {
        pressed: firstHandleDh.dragging
        handle: firstHandle
        value: from

        Component.onCompleted: {
            setMaxValue(firstValueCeil);
        }

        onValueChanged: {
            // when both values clamping to bottom
            //! Set position and visualPosition
            setPosition(Math.max(0, Math.min(1, (value - from) / Math.abs(to - from))));

            //! Set min value of second handler
            second.setMinValue(Math.max(secondValueFloor, value + difference));
        }
    }

    //! Second handle data
    property RangeSliderHandleData second: RangeSliderHandleData {
        pressed: secondHandleDh.dragging
        handle: secondHandle
        value: to

        Component.onCompleted: {
            setMinValue(secondValueFloor);
        }

        onValueChanged: {
            //! Set position and visualPosition
            setPosition(Math.max(0, Math.min(1, (value - from) / Math.abs(to - from))));

            //! Set max value of first handler
            first.setMaxValue(Math.min(firstValueCeil, value - difference));
        }
    }

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: 400
    implicitHeight: 200
    leftInset: 0
    rightInset: 0
    topInset: 0
    bottomInset: 0
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
            color: _control.enabled ? "#0097cd" : Qt.darker("#0097cd", _control.darkerShade)
        }

        Rectangle {
            y: (parent.shapeHeight + 6)
            x: (parent.shapeWidth + (parent.width - parent.shapeWidth) / 2 - width)
            width: parent.pathWidth
            height: width
            radius: width / 2
            color: _control.enabled ? "#ea0600" : Qt.darker("#ea0600", _control.darkerShade)
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
                fillGradient: ConicalGradient {
                    angle: -10
                    centerX: _control.background.shapeWidth / 2
                    centerY: _control.background.shapeHeight

                    GradientStop {
                        position: 0
                        color: true ? "#ea0600" : Qt.darker("#ea0600", _control.darkerShade)
                    }

                    GradientStop {
                        position: 0.55
                        color: true ? "#0097cd" : Qt.darker("#0097cd", _control.darkerShade)
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
                startX: 0
                startY: background.shapeHeight
                capStyle: ShapePath.RoundCap
                fillColor: "transparent"
                strokeWidth: background.pathWidth
                strokeColor: showGreySection ? Qt.darker(Style.accent, 1.2) : "transparent"

                PathAngleArc {
                    centerX: background.shapeWidth / 2
                    centerY: background.shapeHeight
                    radiusX: (background.shapeWidth - background.pathWidth) / 2
                    radiusY: background.shapeHeight - background.pathWidth / 2
                    startAngle: 180 + firstHandle.rotation
                    sweepAngle: secondHandle.rotation - firstHandle.rotation
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
        id: firstHandle

        readonly property real angleRange: 180.0

        parent: _control.background
        x: parent.width / 2
        y: parent.height - (_control.height - _control.background.shapeHeight) / 2
        rotation: {
            var valueRange = Math.abs(to - from);
            return (Math.max((first.value - from) / (valueRange > 0 ? valueRange : 1), 0) * angleRange) % 360
        }
        opacity: secondHandleDh.dragging ? 0.65 : 1.

        Rectangle {
            id: firstHandleCircle
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
            id: firstHandleDh
            property bool dragging: false
            property real startAngle: -1

            readonly property real maxThresholdRadiusSq: Math.pow(_control.background.shapeHeight + 24, 2)
            readonly property real minThresholdRadiusSq: Math.pow(_control.background.shapeHeight
                                                                  - _control.background.pathWidth - 24, 2)
            readonly property point center: Qt.point((parent.width + parent.x) / 2, parent.height + parent.y * 2)

            enabled: !secondHandleDh.dragging
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

                        if (Math.abs(pressAngle - firstHandle.rotation) < 14) {
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
                    var newValue = first.value + diffAngle / (firstHandle.angleRange) * Math.abs(to - from);
                    first.setValue(Math.max(from, Math.min(firstValueCeil, to, newValue)));


                    startAngle = angle;
                }
            }
        }

        //! This MouseArea is added so touch/press events that won't start dragging in DragHandler
        //! can be passed to other Items like SwipeView or Flickable
        MouseArea {
            enabled: !firstHandleDh.dragging
            width: firstHandleDh.dragging ? 0 : parent.width
            height: firstHandleDh.dragging ? 0 : parent.height
            anchors.centerIn: parent
            propagateComposedEvents: true
        }
    }

    Item {
        id: secondHandle

        readonly property real angleRange: 180.0

        parent: _control.background
        x: parent.width / 2
        y: parent.height - (_control.height - _control.background.shapeHeight) / 2
        rotation: {
            var valueRange = Math.abs(to - from);
            return (((second.value - from) / (valueRange > 0 ? valueRange : 1)) * angleRange) % 360
        }
        opacity: firstHandleDh.dragging ? 0.65 : 1.

        Rectangle {
            id: secondHandleCircle
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
            id: secondHandleDh
            property bool dragging: false
            property real startAngle: -1

            readonly property real maxThresholdRadiusSq: Math.pow(_control.background.shapeHeight + 24, 2)
            readonly property real minThresholdRadiusSq: Math.pow(_control.background.shapeHeight
                                                                  - _control.background.pathWidth - 24, 2)
            readonly property point center: Qt.point((parent.width + parent.x) / 2, parent.height + parent.y * 2)

            enabled: !firstHandleDh.dragging
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

                        if (Math.abs(pressAngle - secondHandle.rotation) < 14) {
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
                    var newValue = second.value + diffAngle / (secondHandle.angleRange) * Math.abs(to - from);
                    second.setValue(Math.max(secondValueFloor, from, Math.min(to, newValue)));


                    startAngle = angle;
                }
            }
        }

        //! This MouseArea is added so touch/press events that won't start dragging in DragHandler
        //! can be passed to other Items like SwipeView or Flickable
        MouseArea {
            enabled: !secondHandleDh.dragging
            width: secondHandleDh.dragging ? 0 : parent.width
            height: secondHandleDh.dragging ? 0 : parent.height
            anchors.centerIn: parent
            propagateComposedEvents: true
        }
    }
}
