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

            // Skip Internal Sensor Test because it is added at the beginning as an automated test
            //_root.StackView.view.push("qrc:/Stherm/View/Test/InternalSensorTestPage.qml", {
            //                              "uiSession": uiSession
            //                          })

            if (deviceController.startMode !== 0) {
                _root.StackView.view.push("qrc:/Stherm/View/Test/RelayTestPage.qml", {
                                             "uiSession": uiSession
                                         })
            } else {
                //! Test mode enabled with GPIO as there is no ti board connected
                _root.StackView.view.push("qrc:/Stherm/View/Test/QRCodeTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }
        }
    }

    ConfirmPopup {
        id: popup
        closeButtonVisible: false
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
