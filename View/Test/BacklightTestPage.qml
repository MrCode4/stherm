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
    backlightSwitch.checked: true
    // hasShades: false

    // onUnshadedColorChanged: {
        //! Apply selected color to device immediately
    // }

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
                                          "uiSession": uiSession
                                         })
        }
    }

    InfoPopup {
        id: infoPopup
        message: "Backlight test"
        detailMessage: "The screen will change hue,<br>brightness and shades"
        visible: true

        onAccepted: {
            colorTimer.value = 0
            colorTimer.start()
        }
    }

    ConfirmPopup {
        id: confirmPopup1
        closeButtonEnabled: false
        closePolicy: Popup.NoAutoClose
        message: "Backlight test"
        detailMessage: "Was the backlight working as<br>expected when adjusting the values"
        onAccepted: {
            deviceController.deviceControllerCPP.writeTestResult("Backlight test", true)
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
            deviceController.deviceControllerCPP.writeTestResult("Backlight test", false, "The backlight is not functioning properly")
            nextPage()
        }
    }

    Timer {
        id: colorTimer
        interval: 1000
        repeat: true
        running: false

        property int value: 0
        property int intervals: 10

        onTriggered: {
            if (colorTimer.value === colorTimer.intervals + 1)
            {
                colorTimer.stop()
                brightnessTimer.value = 0
                brightnessTimer.start()
                return
            }

            _root.colorSlider.value = colorTimer.value / colorTimer.intervals
            _root.applyOnline()
            colorTimer.value++
        }
    }

    Timer {
        id: brightnessTimer
        interval: 1000
        repeat: true
        running: false

        property int value: 0
        property int intervals: 10

        onTriggered: {
            if (brightnessTimer.value === brightnessTimer.intervals + 1)
            {
                brightnessTimer.stop()
                buttonTimer.btnIndex = 0
                buttonTimer.start()
                return
            }

            _root.brightnessSlider.value = brightnessTimer.value / brightnessTimer.intervals
            _root.applyOnline()
            brightnessTimer.value++
        }
    }

    Timer {
        id: buttonTimer
        interval: 1000
        repeat: true
        running: false

        property int btnIndex: 0

        onTriggered: {
            if (btnIndex === _root.shadeButtons.length)
            {
                buttonTimer.stop()
                confirmPopup1.open()
                return
            }

            _root.shadeButtons[buttonTimer.btnIndex].checked = true
            _root.applyOnline()
            buttonTimer.btnIndex++
        }
    }

    //! Next button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        enabled: false
    }
}
