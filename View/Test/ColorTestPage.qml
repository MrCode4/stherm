import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ColorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Color Test"
    background: Rectangle {
        color: "blue"
    }

    /* Children
     * ****************************************************************************************/
    //! Next button (loads ColorTestPage)
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            //! Load next page
            if (_root.StackView.view) {
                _root.StackView.view.push("qrc:/Stherm/View/Test/BacklightTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }
        }
    }

    TapHandler {
        onTapped: {
            switch(_root.state) {
            case "black":
                _root.state = "black";
                break;
            case "white":
                _root.state = "white";
                break;
            case "blue":
                _root.state = "green";
                break;
            case "green":
                _root.state = "red";
                break;
            case "red":
                _root.state = "blue";
                break;
            }
        }
    }

    /* States and Transitions
     * ****************************************************************************************/
    state: "blue"
    states: [
        State {
            name: "black"
            PropertyChanges {
                target: _root.background
                color: "black"
            }
        },
        State {
            name: "white"
            PropertyChanges {
                target: _root.background
                color: "white"
            }
        },
        State {
            name: "blue"
            PropertyChanges {
                target: _root.background
                color: "blue"
            }
        },
        State {
            name: "green"
            PropertyChanges {
                target: _root.background
                color: "green"
            }
        },
        State {
            name: "red"
            PropertyChanges {
                target: _root.background
                color: "red"
            }
        }
    ]

    transitions: Transition {
        ColorAnimation {
            target: _root.background
            duration: 400
        }
    }
}
