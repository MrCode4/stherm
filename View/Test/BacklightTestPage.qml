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

    onVisibleChanged: {
        if (visible) {
            infoPopup.open()
        }
    }

    /* Children
     * ****************************************************************************************/

    function nextPage() {
        if (_root.StackView.view) {
            // _root.StackView.view.push("qrc:/Stherm/View/Test/AudioTestPage.qml", {
            //                               "uiSession": uiSession
            //                           })

            _root.StackView.view.push("qrc:/Stherm/View/Test/InternalSensorTestPage.qml", {
                                          "uiSession": uiSession,
                                          "backButtonVisible" : backButtonVisible
                                      })
        }
    }

    InfoPopup {
        id: infoPopup
        message: "Backlight test"
        detailMessage: "The screen will change hue,<br>brightness and shades"
        visible: true

        onAccepted: {
            buttonTimer.btnIndex = 0
            buttonTimer.start()
            buttonTimer.triggered()
        }
    }

    ConfirmPopup {
        id: confirmPopup1
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Backlight test"
        detailMessage: "Are all LEDS on,<br>at the same brightness and colour?"
        onAccepted: {
            backButtonVisible = false;
            deviceController.deviceControllerCPP.saveTestResult("Backlight test", true)
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
        message: "Backlight test"
        detailMessage: "Retry test?"
        onAccepted: {
            infoPopup.open()
        }
        onRejected: {
            backButtonVisible = true;
            deviceController.deviceControllerCPP.saveTestResult("Backlight test", false, "The backlight is not functioning properly")
            nextPage()
        }
    }

    //! Timer to update the shade index and apply the model.
    Timer {
        id: buttonTimer
        interval: 2000
        repeat: true
        running: false

        property int btnIndex: 0

        onTriggered: {
            if (btnIndex === 1) {
                buttonTimer.stop()
                confirmPopup1.open()
                return
            }

            _root.setCurrentColor(btnIndex)
            backlightSwitch.checked = true;
            onlineTimer.startTimer();
            buttonTimer.btnIndex++
        }
    }

    //! Next button, disabled for now, kept for consistency
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        onClicked: nextPage()

        enabled: false
    }
}
