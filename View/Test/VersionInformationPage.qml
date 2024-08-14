import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * VersionInformationPage shows the uid and software version in test mode
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Object properties
     * ****************************************************************************************/
    title: "Version Info"

    // Start the test mode
    Component.onCompleted: {
        timer.start();
    }

    /* Children
     * ****************************************************************************************/

    //! Next button (loads StartTestPage)
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }
        onClicked: {
            //! Load next page
            timer.stop();
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/Test/TestsHostPage.qml", {
                                             "uiSession": uiSession,
                                             "backButtonVisible" : backButtonVisible
                                         })
            }
        }
    }

    ListView {

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true

        model: [
            {"key": "Uid",               "value": deviceController.deviceControllerCPP.deviceAPI.uid},
            {"key": "Software version",  "value": Application.version},
            {"key": "kernel build",  "value": deviceController.deviceControllerCPP.system.kernelBuildVersion()},
            {"key": "rootfs build",  "value": deviceController.deviceControllerCPP.system.rootfsBuildTimestamp()},
            {"key": "TI HW",  "value": deviceController.deviceControllerCPP.hwTI},
            {"key": "TI SW",  "value": deviceController.deviceControllerCPP.swTI},
            {"key": "nRF HW",  "value": deviceController.deviceControllerCPP.hwNRF},
            {"key": "nRF SW",  "value": deviceController.deviceControllerCPP.swNRF},
        ]

        delegate: Item {
            id: mainDelegate

            property real keyWidth: fontMetrics.boundingRect("Software version :").width + leftPadding + rightPadding
            property bool isRowLayout: (width - keyWidth) >= fontMetrics.boundingRect(modelData.value).width

            width: ListView.view.width
            height: Style.delegateHeight * (isRowLayout ? 0.8 : 1.6)
            GridLayout {
                id: textContent

                columnSpacing: 16
                columns: mainDelegate.isRowLayout ? 2 : 1
                rows: mainDelegate.isRowLayout ? 1 : 2

                anchors.fill: parent

                Label {
                    Layout.preferredWidth: mainDelegate.keyWidth
                    font.bold: true
                    text: modelData.key + ":"
                }

                Label {
                    Layout.fillWidth: true
                    font.pointSize: Application.font.pointSize * 0.9
                    textFormat: Text.RichText
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.value.length > 0 ? modelData.value : "Unknown"

                }
            }
        }
    }

    FontMetrics {
        id: fontMetrics
    }

    Timer {
        id: timer

        running: false
        repeat: false
        interval: 5000

        onTriggered: {
            root.StackView.view.push("qrc:/Stherm/View/Test/TestsHostPage.qml", {
                                         "uiSession": Qt.binding(() => uiSession)
                                     });
        }
    }
}
