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

    onVisibleChanged: {
        if (visible) {
            infoPopup.open()
        }
    }

    /* Children
     * ****************************************************************************************/

    function nextPage() {
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/Test/BrightnessTestPage.qml", {
                                         "uiSession": uiSession,
                                         "backButtonVisible" : backButtonVisible
                                     })
        }
    }

    InfoPopup {
        id: infoPopup
        message: "Color test"
        detailMessage: "The screen will change colors<br>between blue, green,<br>red and black"
        visible: true

        onAccepted: {
            root.state = "black"
            timer.start()
            timer.colorIndex = 0
            timer.triggered()
        }
    }

    ConfirmPopup {
        id: confirmPopup1
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Color test"
        detailMessage: "Did you see any dicoloration<br>or dead pixels?"
        onAccepted: {
            confirmPopup2.open()
        }
        onRejected: {
            backButtonVisible = false;
            deviceController.deviceControllerCPP.writeTestResult("Color test", true)
            nextPage()
        }
    }

    ConfirmPopup {
        id: confirmPopup2
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Color test"
        detailMessage: "Retry test?"
        onAccepted: {
            infoPopup.open()
        }
        onRejected: {
            backButtonVisible = true;
            deviceController.deviceControllerCPP.writeTestResult("Color test", false, "The display is discolored or has dead pixels")
            nextPage()
        }
    }

    //! Next button, disabled for now, kept for consistency
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        onClicked: nextPage()
        enabled: false
    }

    Timer {
        id: timer
        interval: 1000
        repeat: true
        running: false

        property var colors: ["blue", "green", "red", "black"]
        property int colorIndex: 0

        onTriggered: {
            root.state = colors[colorIndex]
            colorIndex++

            if (colorIndex === colors.length) {
                timer.stop()
                timer2.start()
            }
        }
    }

    Timer {
        id: timer2
        interval: 5000

        repeat: false
        running: false

        onTriggered: confirmPopup1.open()
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
