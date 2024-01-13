import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * TouchTestPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Test Touch"

    onVisibleChanged: {
        if (visible)
            deviceController.deviceControllerCPP.system.testMode = true;
    }

    /* Children
     * ****************************************************************************************/

    property string pointsState : "0,0,0,0,0,0,0,0,0"

    property var points: ({})

    //! Next button (loads ColorTestPage)
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        enabled: pointsState === "1,1,1,1,1,1,1,1,1"
        onClicked: {
            //! Load next page
            if (_root.StackView.view) {
                _root.StackView.view.push("qrc:/Stherm/View/Test/ColorTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }
        }
    }

    GridLayout {
        anchors.fill: parent
        columns: 3
        columnSpacing: 0
        rowSpacing: 0

        Repeater {
            model: pointsState.split(",")
            delegate: Rectangle {
                Layout.preferredWidth:  40
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignCenter
                radius: width / 2
                color: modelData === "1" ? "green" : "red"

                // seems to be the latest one to be called
                onRadiusChanged:
                {
                   _root.points[index] = Qt.point(x + radius, y + radius)
                }
            }
        }
    }

    //! TapHandler
    TapHandler {
        onTapped: function(point, button) {
            _tappedItem.x = point.position.x;
            _tappedItem.y = point.position.y;
            _tapAnima.restart();
        }
    }

    //! A Rectangle to show tapped point
    Item {
        id: _tappedItem

        Rectangle {
            id: _tappedRect
            anchors.centerIn: parent
            width: 0
            height: width

            //            color: "transparent"
            radius: width / 2
            //            border.width: 2
            color: Qt.alpha(Style.accent, 0.4)

            Rectangle {
                anchors.centerIn: parent
                width: parent.width / 1.6
                height: width

                radius: width / 2

                color: "transparent"//parent.border.color
            }
        }
    }

    //! An animation to show tapped point
    ParallelAnimation {
        id: _tapAnima
        loops: 1

        NumberAnimation {
            target: _tappedRect
            property: "opacity"
            to: 1
            duration: 200
        }

        NumberAnimation {
            target: _tappedRect
            property: "width"
            from: 0
            to: 100
            duration: 500
        }

        SequentialAnimation {
            PauseAnimation { duration: 200 }

            NumberAnimation {
                target: _tappedRect
                property: "opacity"
                from: 1
                to: 0
                duration: 300
            }
        }
    }

    //! Drag line
    Shape {
        id: _shape

        property var points: []

        anchors.fill: parent

        ShapePath {
            startX: 0
            startY: 0
            fillColor: "transparent"
            strokeColor: "white"
            strokeWidth: 4

            PathPolyline {
                path: _shape.points
            }
        }
    }

    //! Drag
    DragHandler {
        readonly property int newPointThershold: 30
        property vector2d lastActiveTranslation

        target: null
        onGrabChanged: function(grabState, point){
            //! See enum QPointingDevice::GrabTransition
            if (grabState === 1) {
                _shape.points.length = 0;

                for(var i = 0; i < 9; i++) {
                    var pos_ = _root.points[i];
                    if (Math.abs(pos_.x - point.position.x) < 40 &&Math.abs(pos_.y - point.position.y) < 40){
                        var states = pointsState.split(",");
                        states[i] = "1";
                        pointsState =  states.join(",")
                    }
                }

                _shape.points.push(Qt.point(point.position.x, point.position.y));

                lastActiveTranslation = Qt.vector2d(0, 0);
                _shape.pointsChanged();
            } else if (grabState === 2 || grabState === 0x30 || grabState === 0x20) {
                //! Play destroy animation
                _clearPathAnima.running = true;
            }
        }

        onActiveTranslationChanged: {
            //! Calculate diff to last point
            if (Math.abs(activeTranslation.x - lastActiveTranslation.x) > newPointThershold
                    || Math.abs(activeTranslation.y - lastActiveTranslation.y) > newPointThershold) {
                //! Add activeTranslation to 'first point' (not last one)
                var firstPoint = _shape.points[0];
                for(var i = 0; i < 9; i++) {
                    var pos_ = _root.points[i];
                    if (Math.abs(pos_.x - (firstPoint.x + activeTranslation.x)) < 40 &&Math.abs(pos_.y - (firstPoint.y + activeTranslation.y)) < 40)
                    {
                        var states = pointsState.split(",");
                        states[i] = "1";
                        pointsState =  states.join(",")
                    }
                }
                _shape.points.push(Qt.point(firstPoint.x + activeTranslation.x,
                                            firstPoint.y + activeTranslation.y/* - _root.header.height*/));

                lastActiveTranslation = activeTranslation;
                _shape.pointsChanged();
            }
        }
    }

    //! An animation to clear drag path
    NumberAnimation {
        id: _clearPathAnima
        target: _shape
        loops: 1
        running: false
        property: "opacity"
        duration: 300
        to: 0
        onFinished: {
            _shape.points = [];
            //! Restore opacity
            _shape.opacity = 1.;
        }
    }
}
