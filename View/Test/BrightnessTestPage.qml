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
    title: "Brightness Test"
    headerColor: "black"
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
            root.StackView.view.push("qrc:/Stherm/View/Test/BacklightTestPage.qml", {
                                          "uiSession": uiSession
                                      })
        }
    }

    InfoPopup {
        id: infoPopup
        message: "Brightness test"
        detailMessage: "The Brightness level will vary from low to high every 3 seconds."
        visible: true

        onAccepted: {
            timer.start()
        }
    }

    ConfirmPopup {
        id: confirmPopup1
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Brightness test"
        detailMessage: "Did you see any issue in low light or high light?"
        onAccepted: {
            confirmPopup2.open()
        }
        onRejected: {
            deviceController.deviceControllerCPP.writeTestResult("Brightness test", true)
            nextPage()
        }
    }

    ConfirmPopup {
        id: confirmPopup2
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Brightness test"
        detailMessage: "Retry test?"
        onAccepted: {
            infoPopup.open()
        }
        onRejected: {
            deviceController.deviceControllerCPP.writeTestResult("Brightness test", false, "The display is discolored or has dead pixels")
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

        onClicked: {
            deviceController.deviceControllerCPP.stopTestBrightness()
            timer.stop()
            confirmPopup1.open()
        }
    }

    Timer {
        id: timer
        interval: 100
        repeat: true
        running: false

        property int brightness: 5

        onTriggered: {
            if (brightness > 255)
                brightness = 5
            else
                brightness += 9

            deviceController.deviceControllerCPP.testBrightness(brightness)
        }
    }
}
