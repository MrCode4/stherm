import QtQuick

import Ronia
import Ronia.impl
import Stherm
import "./impl"

/*!
 * LimitedRangeSlider
 */
Control {
    id: control

    property int orientation: Qt.Horizontal
    readonly property bool horizontal: orientation === Qt.Horizontal
    property alias first: internal.first
    property alias second: internal.second

    QtObject {
        id: internal

        property RangeSliderHandleData first: RangeSliderHandleData {
            implicitHandleHeight: 24
            implicitHandleWidth: 24
            position: 0.
            handle: Rectangle {
                anchors {
                    verticalCenter: control.horizontal ? (parent?.verticalCenter ?? undefined) : undefined
                    horizontalCenter: control.horizontal ? undefined : (parent?.horizontalCenter ?? undefined)
                }
                x: control.horizontal ? control.first.position * parent.width - width / 2 : 0
                y: control.horizontal ? 0 : control.first.position * parent.height - width / 2
                parent: control.contentItem
                width: control.first.implicitHandleWidth
                height: control.first.implicitHandleHeight
                radius: width / 2
                opacity: 0.5

                DragHandler {
                    //! Stores previous position value
                    property real prevPosition

                    xAxis.enabled: control.horizontal
                    xAxis.maximum: control.second.position * control.contentItem.width - target.width / 2
                    xAxis.minimum: -target.width / 2

                    yAxis.enabled: control.horizontal
                    yAxis.maximum: control.second.position * control.contentItem.width - target.width / 2
                    yAxis.minimum: -target.width / 2

                    onGrabChanged: function(grab, point) {
                        if (grab === 1) {
                            prevPosition = control.first.position;
                        }
                    }

                    onActiveTranslationChanged: {
                        if (control.horizontal) {
                            var newPos = Math.max(0, Math.min(control.second.position,
                                                              prevPosition + (activeTranslation.x / control.contentItem.width)
                                                              )
                                                  );
                            if (control.first.position !== newPos) {
                                control.first.position = newPos;
                            }
                        }
                    }
                }
            }
        }

        property RangeSliderHandleData second: RangeSliderHandleData {
            implicitHandleHeight: 24
            implicitHandleWidth: 24
            position: 1.
            handle: Rectangle {
                anchors {
                    verticalCenter: control.horizontal ? (parent?.verticalCenter ?? undefined) : undefined
                    horizontalCenter: control.horizontal ? undefined : (parent?.horizontalCenter ?? undefined)
                }
                x: control.horizontal ? control.second.position * parent.width - width / 2 : 0
                y: control.horizontal ? 0 : control.second.position * parent.height - width / 2
                parent: control.contentItem
                width: control.first.implicitHandleWidth
                height: control.first.implicitHandleHeight
                radius: width / 2
                opacity: 0.5

                DragHandler {
                    //! Stores previous position value
                    property real prevPosition

                    xAxis.enabled: control.horizontal
                    xAxis.maximum: control.contentItem.width - target.width / 2
                    xAxis.minimum: control.first.position * control.contentItem.width - target.width / 2

                    yAxis.enabled: control.horizontal
                    yAxis.maximum: control.contentItem.height - target.width / 2
                    yAxis.minimum: control.first.position * control.contentItem.width - target.width / 2

                    onGrabChanged: function(grab, point) {
                        if (grab === 1) {
                            prevPosition = control.second.position;
                        }
                    }

                    onActiveTranslationChanged: {
                        if (control.horizontal) {
                            var newPos = Math.max(control.first.position, Math.min(1,
                                                              prevPosition + (activeTranslation.x / control.contentItem.width)
                                                              )
                                                  );
                            if (control.second.position !== newPos) {
                                control.second.position = newPos;
                            }
                        }
                    }
                }
            }
        }
    }

    background: Rectangle {
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        implicitWidth: control.horizontal ? 200 : 48
        implicitHeight: control.horizontal ? 48 : 200
        width: control.horizontal ? control.availableWidth : 4
        height: control.horizontal ? 4 : control.availableHeight
        scale: control.horizontal && control.mirrored ? -1 : 1
        color: control.enabled ? Qt.alpha(Style.accent, 0.33) : control.Material.sliderDisabledColor

        Rectangle {
            x: control.horizontal ? control.first.position * parent.width : 0
            y: control.horizontal ? 0 : control.second.visualPosition * parent.height
            width: control.horizontal ? control.second.position * parent.width - control.first.position * parent.width : 4
            height: control.horizontal ? 4 : control.second.position * parent.height - control.first.position * parent.height

            color: control.enabled ? control.Material.accentColor : control.Material.sliderDisabledColor
        }
    }

    contentItem: Item {
        Repeater {
            model: ObjectModel {
                Component.onCompleted: {
                    append(first.handle);
                    append(second.handle)
                }
            }
        }
    }
}
