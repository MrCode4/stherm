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


    TapHandler {
        onTapped: {
            switch(_root.state) {
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
