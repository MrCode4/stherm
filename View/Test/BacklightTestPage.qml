import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BacklightTestPage
 * ***********************************************************************************************/
BacklightPage {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    topPadding: 0
    bottomPadding: 32 * scaleFactor
    title: "Backlight Test"
    isTest: true
    // hasShades: false

    // onUnshadedColorChanged: {
        //! Apply selected color to device immediately
    // }

    /* Children
     * ****************************************************************************************/

    function nextPage() {
        if (_root.StackView.view) {
            // _root.StackView.view.push("qrc:/Stherm/View/Test/AudioTestPage.qml", {
            //                               "uiSession": uiSession
            //                           })

            _root.StackView.view.push("qrc:/Stherm/View/Test/InternalSensorTestPage.qml", {
                                          "uiSession": uiSession
                                         })
        }
    }

    ConfirmPopup {
        id: popup
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Backlight test"
        detailMessage: "Was the backlight working as<br>expected when adjusting the values"
        onAccepted: {
            deviceController.deviceControllerCPP.writeTestResult("Backlight test", true)
            nextPage()
        }
        onRejected: {
            deviceController.deviceControllerCPP.writeTestResult("Backlight test", false, "The backlight is not functioning properly")
            nextPage()
        }
    }

    //! Next button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        onClicked: popup.open()
    }
}
