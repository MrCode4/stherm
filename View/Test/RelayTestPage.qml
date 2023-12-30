import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BacklightTestPage
 * ***********************************************************************************************/
WiringPage {
    id: _root

    /* Object properties
     * ****************************************************************************************/

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
    //! Next button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        onClicked: {
            if (_root.StackView.view) {
                //! Next page
//                _root.StackView.view.push("qrc:/Stherm/View/Test/", {
//                                              "uiSession": uiSession
//                                          })
                //! Finish test
            }
        }
    }
}
