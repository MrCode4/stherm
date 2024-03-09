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

    Component.onCompleted: timer.start()

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
                root.StackView.view.push("qrc:/Stherm/View/Test/StartTestPage.qml", {
                                              "uiSession": uiSession
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
            {"key": "TI HW",  "value": deviceController.deviceControllerCPP.getTI_HW()},
            {"key": "TI SW",  "value": deviceController.deviceControllerCPP.getTI_SW()},
            {"key": "nRF HW",  "value": deviceController.deviceControllerCPP.getNRF_HW()},
            {"key": "nRF SW",  "value": deviceController.deviceControllerCPP.getNRF_SW()},
        ]

        delegate: Item {
            visible: modelData.value.length > 0
            width: ListView.view.width
            height: Style.delegateHeight * 0.8
            RowLayout {
                id: textContent
                spacing: 16

                anchors.fill: parent

                Label {
                    Layout.preferredWidth: fontMetrics.boundingRect("Software version :").width + leftPadding + rightPadding
                    font.bold: true
                    text: modelData.key + ":"
                }

                Label {
                    Layout.fillWidth: true
                    font.pointSize: Application.font.pointSize * 0.9
                    textFormat: Text.RichText
                    horizontalAlignment: Text.AlignLeft
                    text: modelData.value

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
        interval: 10000

        onTriggered: {
            root.StackView.view.push("qrc:/Stherm/View/Test/StartTestPage.qml", {
                                         "uiSession": Qt.binding(() => uiSession)
                                     });
        }
    }
}
