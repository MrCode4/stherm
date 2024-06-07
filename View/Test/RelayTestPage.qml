import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * RelayTestPage
 * ***********************************************************************************************/
WiringPage {
    id: root

    /* Object properties
     * ****************************************************************************************/

    Component.onDestruction: {
        // Back to last relays
        deviceController.deviceControllerCPP.sendRelaysBasedOnModel();
    }

    topPadding: 0
    bottomPadding: 32 * scaleFactor
    title: "Relay Test"

    wiring: Wiring {
        onIsRChanged:   deviceController.testRelays(wiring);
        onIsCChanged:   deviceController.testRelays(wiring);
        onIsGChanged:   deviceController.testRelays(wiring);
        onIsY1Changed:  deviceController.testRelays(wiring);
        onIsY2Changed:  deviceController.testRelays(wiring);
        onIsT2Changed:  deviceController.testRelays(wiring);
        onIsW1Changed:  deviceController.testRelays(wiring);
        onIsW2Changed:  deviceController.testRelays(wiring);
        onIsW3Changed:  deviceController.testRelays(wiring);
        onIsOBChanged:  deviceController.testRelays(wiring);
        onIsT1pChanged: deviceController.testRelays(wiring);
        onIsT1nChanged: deviceController.testRelays(wiring);
    }

    /* Children
     * ****************************************************************************************/

    onVisibleChanged: {
        if (visible) {
            wiring.isR   = false
            wiring.isC   = false
            wiring.isG   = false
            wiring.isY1  = false
            wiring.isY2  = false
            wiring.isT2  = false
            wiring.isW1  = false
            wiring.isW2  = false
            wiring.isW3  = false
            wiring.isOB  = false
            wiring.isT1p = false
            wiring.isT1n = false
            infoPopup.open()
        }
    }

    InfoPopup {
        id: infoPopup
        message: "Relay test"
        detailMessage: "The Relays will switch<br>one by one every second."
        visible: true

        onAccepted: {
            testTimer.counter = 0;
            testTimer.finalStep = false;
            testTimer.start()
        }
    }

    function nextPage() {
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/Test/QRCodeTestPage.qml", {
                                         "uiSession": uiSession,
                                         "backButtonVisible" : backButtonVisible
                                     })
        }
    }

    Timer {
        id: testTimer

        property int counter : 0
        property bool finalStep : false

        interval: 100
        repeat: true
        running: false

        onTriggered: {
            counter = counter + 2;

            if (counter < 3) {
                wiring.isR = !wiring.isR
            } else if (counter < 5) {
                wiring.isC = !wiring.isC
            } else if (counter < 7) {
                wiring.isG = !wiring.isG
            } else if (counter < 9) {
                wiring.isY1 = !wiring.isY1
            } else if (counter < 11) {
                wiring.isY2 = !wiring.isY2
            } else if (counter < 13) {
                wiring.isT2 = !wiring.isT2
            } else if (counter < 15) {
                wiring.isW1 = !wiring.isW1
            } else if (counter < 17) {
                wiring.isW2 = !wiring.isW2
            } else if (counter < 19) {
                wiring.isW3 = !wiring.isW3
            } else if (counter < 21) {
                wiring.isOB = !wiring.isOB
            } else if (counter < 23) {
                wiring.isT1p = !wiring.isT1p
            } else if (counter < 25) {
                wiring.isT1n = !wiring.isT1n
            } else {
                // finished all relays
                stop();

                if (finalStep)
                    confirmPopup1.open();
                else
                    testTimerReset.start();
            }
        }
    }

    Timer {
        id: testTimerReset

        interval: 2000
        repeat: false
        running: false

        onTriggered: {
            testTimer.counter = 0;
            testTimer.finalStep = true;
            testTimer.start()
        }
    }

    ConfirmPopup {
        id: confirmPopup1
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Relay test"
        detailMessage: "Are all relay LEDs switching?"
        onAccepted: {
            backButtonVisible = false;
            deviceController.deviceControllerCPP.writeTestResult("Relay test", true)
            nextPage()
        }
        onRejected: {
            confirmPopup2.open()
        }
    }

    ConfirmPopup {
        id: confirmPopup2
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Relay test"
        detailMessage: "Retry test?"
        onAccepted: {
            infoPopup.open()
        }
        onRejected: {
            backButtonVisible = true;
            deviceController.deviceControllerCPP.writeTestResult("Relay test", false, "The backlight is not functioning properly")
            nextPage()
        }
    }

    //! Next button
   ToolButton {
       parent: root.header.contentItem
       contentItem: RoniaTextIcon {
           text: FAIcons.arrowRight
       }

       onClicked: {
           testTimer.stop();
           testTimerReset.stop()
           confirmPopup1.open();
       }
   }
}
