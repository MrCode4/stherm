import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ColorTestPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Color test"
    background: Rectangle {
        color: "black"
    }

    /* Children
     * ****************************************************************************************/
    //! Next button (loads ColorTestPage)
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            //! Load next page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/Test/BacklightTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }
        }
    }

    TapHandler {
        onTapped: {
            switch(root.state) {
            case "black":
                root.state = "white";
                break;
            case "white":
                root.state = "blue";
                break;
            case "blue":
                root.state = "green";
                break;
            case "green":
                root.state = "red";
                break;
            case "red":
                root.state = "black";
                break;
            }
        }
    }

    /* States and Transitions
     * ****************************************************************************************/
    state: "black"
    states: [
        State {
            name: "black"
            PropertyChanges {
                target: root.background
                color: "black"
            }
        },
        State {
            name: "white"
            PropertyChanges {
                target: root.background
                color: "white"
            }
        },
        State {
            name: "blue"
            PropertyChanges {
                target: root.background
                color: "blue"
            }
        },
        State {
            name: "green"
            PropertyChanges {
                target: root.background
                color: "green"
            }
        },
        State {
            name: "red"
            PropertyChanges {
                target: root.background
                color: "red"
            }
        }
    ]

    transitions: Transition {
        ColorAnimation {
            target: root.background
            duration: 400
        }
    }
}
