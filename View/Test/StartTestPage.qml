import QtQuick
import QtQuick.Shapes
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Start Test Page
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property int testCounter: 0
    property int allTests:    4

    //! System, use in update notification
    property System                 system:           deviceController.deviceControllerCPP.system

    /* Object properties
     * ****************************************************************************************/
    title: "Start Test"

    Component.onCompleted: {
        deviceController.deviceControllerCPP.system.testMode = true;
        deviceController.deviceControllerCPP.beginTesting()
    }

    /* Children
     * ****************************************************************************************/

    Timer {
        interval: 1000
        repeat: false
        running: true

        onTriggered: {
            // Test 1
            if (system.installUpdateService()) {
                testCounter++;
                deviceController.deviceControllerCPP.writeTestResult("Update service", true)
            } else {
                notPassedTests.text += "\nThe Update service can not be installed."
                deviceController.deviceControllerCPP.writeTestResult("Update service", false, "The Update service can not be installed")
            }

            // Test 2
            if (system.mountUpdateDirectory()) {
                testCounter++;
                deviceController.deviceControllerCPP.writeTestResult("Mount update directory", true)
            } else {
                notPassedTests.text += "\nThe Update directory can not be mounted."
                deviceController.deviceControllerCPP.writeTestResult("Mount update directory", false, "The Update directory can not be mounted")
            }

            // Test 3
            if (system.mountRecoveryDirectory()) {
                testCounter++;
                deviceController.deviceControllerCPP.writeTestResult("Mount recovery directory", true)
            } else {
                notPassedTests.text += "\nThe Recovery directory can not be mounted."
                deviceController.deviceControllerCPP.writeTestResult("Mount update directory", false, "The Recovery directory can not be mounted")
            }

            // Test 4 (NRF Version)
            if (deviceController.deviceControllerCPP.checkNRFFirmwareVersion()) {
                testCounter++;
                deviceController.deviceControllerCPP.writeTestResult("NRF compatibility", true)
            } else {
                notPassedTests.text += "\nThe nrf version and the app version are not compatible."
                deviceController.deviceControllerCPP.writeTestResult("NRF compatibility", false, "The nrf version and the app version are not compatible")
            }
        }
    }

    Timer {
        id: nextPageTimer
        interval: 10000
        repeat: false
        running: testCounter === allTests

        onTriggered: {
            nextPage();
        }
    }

    //! Next button (loads TouchTestPage)
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        enabled: testCounter === allTests
        onClicked: {
            nextPage();
        }
    }

    Text {
        id: counterTestText

        anchors.centerIn: parent

        color: Style.foreground
        horizontalAlignment: Text.AlignHCenter
        text: testCounter + (testCounter > 1 ? " tests" : " test") + " passed..."
    }

    Text {
        id: notPassedTests

        anchors.top: counterTestText.bottom
        anchors.topMargin: 5
        anchors.left: parent.left
        anchors.right: parent.right

        wrapMode: Text.WordWrap
        color: Style.foreground
        horizontalAlignment: Text.AlignHCenter
    }

    function nextPage() {
        nextPageTimer.stop()
        //! Load next page
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/Test/TouchTestPage.qml", {
                                         "uiSession": uiSession
                                     })
        }
    }
}
