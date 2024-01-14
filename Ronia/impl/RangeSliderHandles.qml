import QtQuick

import Ronia
import Ronia.impl

Item {
    id: sliderHandles

    property bool horizontal
    property real to: 0
    property real from: 0
    property real difference: 0
    property color handleColor

    property QtObject first: QtObject {
        property Item handle: firstHandle
        property alias value: firstValue.value
        readonly property bool hovered: false
        readonly property bool pressed: firstDrag.active
        readonly property real position: value / Math.abs(sliderHandles.to - sliderHandles.from)
        readonly property real visualPosition: 0.
        readonly property real implicitHandleHeight: handle.implicitHeight
        readonly property real implicitHandleWidth: handle.implicitWidth
    }

    property QtObject second: QtObject {
        property Item handle: secondHandle
        property alias value: secondValue.value
        readonly property bool hovered: false
        readonly property bool pressed: secondDrag.active
        readonly property real position: value / Math.abs(sliderHandles.to - sliderHandles.from)
        readonly property real visualPosition: 1.
        readonly property real implicitHandleHeight: handle.implicitHeight
        readonly property real implicitHandleWidth: handle.implicitWidth
    }

    Item {
        id: firstHandle
        x: sliderHandles.horizontal ? sliderHandles.first.position * parent.width - width / 2 : (parent.width - width) / 2
        y: sliderHandles.horizontal ? (parent.height - height) / 2 : sliderHandles.first.position * parent.height - width / 2
        implicitWidth: 36
        implicitHeight: 36

        BindableQReal {
            id: firstValue
            value: sliderHandles.from
        }

        Rectangle {
            anchors.centerIn: parent
            width: 18
            height: 18
            radius: width / 2
            color: handleColor
        }

        DragHandler {
            id: firstDrag
            //! Stores previous position value
            property real prevPosition

            xAxis.enabled: sliderHandles.horizontal
            xAxis.maximum: (sliderHandles.second.position - sliderHandles.difference) * sliderHandles.width - target.width / 2
            xAxis.minimum: -target.width / 2

            yAxis.enabled: !sliderHandles.horizontal
            yAxis.maximum: (sliderHandles.second.position - sliderHandles.difference) * sliderHandles.width - target.width / 2
            yAxis.minimum: -target.width / 2

            onGrabChanged: function(grab, point) {
                if (grab === 1) {
                    prevPosition = sliderHandles.first.position;
                }
            }

            onActiveTranslationChanged: {
                if (sliderHandles.horizontal) {
                    var newPos = Math.max(0, Math.min(sliderHandles.second.position - sliderHandles.difference,
                                                      prevPosition + (activeTranslation.x / sliderHandles.width)
                                                      )
                                          );

                    var newValue = newPos * (to - from) //! Assuming that to is bigger than from
                    if (firstValue.value !== newValue) {
                        firstValue.setValue(newValue); //! Use setter to avoid breaking bindings
                    }
                }
            }
        }
    }

    Item {
        id: secondHandle

        x: sliderHandles.horizontal ? sliderHandles.second.position * parent.width - width / 2 : (parent.width - width) / 2
        y: sliderHandles.horizontal ? (parent.height - height) / 2 : sliderHandles.second.position * parent.height - width / 2
        implicitWidth: 36
        implicitHeight: 36

        BindableQReal {
            id: secondValue
            value: sliderHandles.to
        }

        Rectangle {
            anchors.centerIn: parent
            width: 18
            height: 18
            radius: width / 2
            color: handleColor
        }

        DragHandler {
            id: secondDrag
            //! Stores previous position value
            property real prevPosition

            xAxis.enabled: sliderHandles.horizontal
            xAxis.maximum: sliderHandles.width - secondHandle.width / 2
            xAxis.minimum: (sliderHandles.first.position + sliderHandles.difference) * sliderHandles.width - target.width / 2

            yAxis.enabled: !sliderHandles.horizontal
            yAxis.maximum: -target.width / 2
            yAxis.minimum: (sliderHandles.second.position + sliderHandles.difference) * sliderHandles.width - target.width / 2

            onGrabChanged: function(grab, point) {
                if (grab === 1) {
                    prevPosition = sliderHandles.second.position;
                }
            }

            onActiveTranslationChanged: {
                if (sliderHandles.horizontal) {
                    var newPos = Math.min(1, Math.max(sliderHandles.first.position + sliderHandles.difference,
                                                      prevPosition + (activeTranslation.x / sliderHandles.width)
                                                      )
                                          );

                    var newValue = newPos * (to - from) //! Assuming that to is bigger than from
                    if (secondValue.value !== newValue) {
                        secondValue.setValue(newValue); //! Use setter to avoid breaking bindings
                    }
                }
            }
        }
    }
}
