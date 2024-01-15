import QtQuick

import Ronia
import Ronia.impl

Item {
    id: sliderHandles

    //! Is slider horizontal
    property bool horizontal

    //! to: must be max of to and from
    property real to: 0

    //! from: must be min of to and from
    property real from: 0

    //! Difference between first and second values
    property real difference: 0

    //! Difference value mapped to position (0 to 1)
    readonly property real positionDifference: (difference - from) / Math.abs(to - from)

    //! Color of handles
    property color handleColor

    //! First handle data
    property RangeSliderHandleData first: RangeSliderHandleData {
        handle: firstHandle
        value: from

        onValueChanged: {
            //! Set position and visualPosition
            setPosition(Math.max(0, Math.min(1, (value - from) / Math.abs(to - from))));

            //! Set min value of second handler
            sliderHandles.second.setMinValue(value + difference);
        }
    }

    //! Second handle data
    property RangeSliderHandleData second: RangeSliderHandleData {
        handle: secondHandle
        value: to

        onValueChanged: {
            //! Set position and visualPosition
            setPosition(Math.max(0, Math.min(1, (value - from) / Math.abs(to - from))));

            //! Set max value of second handler
            sliderHandles.first.setMaxValue(value - difference);
        }
    }

    onToChanged: {
        sliderHandles.second.setMaxValue(to);
        sliderHandles.first.setMaxValue(Math.min(sliderHandles.second.value - difference, to));
    }

    onFromChanged: {
        sliderHandles.second.setMinValue(Math.max(sliderHandles.first.value + difference, from));
        sliderHandles.first.setMinValue(from);
    }

    Item {
        id: firstHandle
        x: sliderHandles.horizontal ? sliderHandles.first.position * parent.width - width / 2 : (parent.width - width) / 2
        y: sliderHandles.horizontal ? (parent.height - height) / 2 : sliderHandles.first.position * parent.height - width / 2
        implicitWidth: 36
        implicitHeight: 36

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
            xAxis.maximum: (sliderHandles.second.position - sliderHandles.difference) * sliderHandles.width - parent.width / 2
            xAxis.minimum: -parent.width / 2

            yAxis.enabled: !sliderHandles.horizontal
            yAxis.maximum: (sliderHandles.second.position - sliderHandles.difference) * sliderHandles.width - parent.width / 2
            yAxis.minimum: -parent.width / 2
            target: null

            onGrabChanged: function(grab, point) {
                if (grab === 1) {
                    prevPosition = sliderHandles.first.position;
                }
            }

            onActiveTranslationChanged: {
                if (sliderHandles.horizontal) {
                    var newPos = Math.max(0, Math.min(sliderHandles.second.position - sliderHandles.positionDifference,
                                                      prevPosition + (activeTranslation.x / sliderHandles.width)
                                                      )
                                          );

                    var newValue = newPos * (to - from) + from; //! Assuming that to is bigger than from
                    if (sliderHandles.first.value !== newValue) {
                        sliderHandles.first.setValue(newValue); //! Use setter to avoid breaking bindings
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
            xAxis.minimum: (sliderHandles.first.position + sliderHandles.difference) * sliderHandles.width - parent.width / 2

            yAxis.enabled: !sliderHandles.horizontal
            yAxis.maximum: -parent.width / 2
            yAxis.minimum: (sliderHandles.second.position + sliderHandles.difference) * sliderHandles.width - parent.width / 2
            target: null

            onGrabChanged: function(grab, point) {
                if (grab === 1) {
                    prevPosition = sliderHandles.second.position;
                }
            }

            onActiveTranslationChanged: {
                if (sliderHandles.horizontal) {
                    var newPos = Math.min(1, Math.max(sliderHandles.first.position + sliderHandles.positionDifference,
                                                      prevPosition + (activeTranslation.x / sliderHandles.width)
                                                      )
                                          );

                    var newValue = newPos * (to - from) + from //! Assuming that to is bigger than from
                    if (sliderHandles.second.value !== newValue) {
                        sliderHandles.second.setValue(newValue); //! Use setter to avoid breaking bindings
                    }
                }
            }
        }
    }
}
