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
    title: "Color Test"
    headerColor:  (root.state === "white") ? "black" : "white"
    background: Rectangle {
        color: "black"
    }

    /* Children
     * ****************************************************************************************/

    function nextPage() {
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/Test/BacklightTestPage.qml", {
                                          "uiSession": uiSession
                                      })
        }
    }

    ConfirmPopup {
        id: popup
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Color test"
        detailMessage: "Did you see any dicoloration<br>or dead pixels?"
        onAccepted: {
            deviceController.deviceControllerCPP.writeTestResult("Color test", false, "The display is discolored or has dead pixels")
            nextPage()
        }
        onRejected: {
            deviceController.deviceControllerCPP.writeTestResult("Color test", true)
            nextPage()
        }
    }

    //! Next button (loads ColorTestPage)
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
            color: headerColor
        }
        onClicked: popup.open()
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
