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

    Timer {
        id: testTimer

        property int counter : 0

        interval: 1000
        repeat: true
        running: true

        onTriggered: {
            counter++;

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
                // we can restart from first if we do not stop
                counter = 1;
            }
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

           //! Next page
           if (root.StackView.view) {
               root.StackView.view.push("qrc:/Stherm/View/Test/QRCodeTestPage.qml", {
                                             "uiSession": uiSession
                                         })
           }
       }
   }
}
