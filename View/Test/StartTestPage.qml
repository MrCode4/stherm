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

    //! System, use in update notification
    property System                 system:           deviceController.deviceControllerCPP.system

    /* Object properties
     * ****************************************************************************************/
    title: "Start Test"

    Component.onCompleted: {
        deviceController.deviceControllerCPP.system.testMode = true;
    }

    /* Childrent
     * ****************************************************************************************/

    Timer {
        interval: 1000
        repeat: false
        running: true

        onTriggered: {
            if (system.installUpdateService()) {
                testCounter++;

            } else {
                notPassedTests.text += "\nThe Update service can not be installed."
            }

            if (system.mountUpdateDirectory()) {
                testCounter++;

            } else {
                notPassedTests.text += "\nThe Update directory can not be mounted."
            }
        }
    }

    //! Next button (loads TouchTestPage)
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        enabled: testCounter === 2
        onClicked: {
            //! Load next page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/Test/TouchTestPage.qml", {
                                              "uiSession": uiSession
                                          })
            }
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

}
