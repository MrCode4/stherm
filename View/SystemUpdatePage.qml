import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemUpdatePage retrieves system update information and prepare device to update.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property delcaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Update"

    /* Children
     * ****************************************************************************************/
    GridLayout {
        height: Math.min(root.availableHeight, implicitHeight)

        anchors.top: parent.top
        columns: 2
        rowSpacing: 16
        columnSpacing: 32

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Last update date: ").width + leftPadding + rightPadding
            font.bold: true
            text: "Last update date: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignLeft
            text: deviceController.deviceControllerCPP.system.latestVersionDate
        }

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Last update date: ").width + leftPadding + rightPadding
            font.bold: true
            text: "Update Available: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignLeft
            text: deviceController.deviceControllerCPP.system.latestVersion
        }

        Rectangle {

            visible: changeLogTextArea.text.length > 0
            height: changeLogTextArea.text.length > 0 ? Math.min(changeLogTextArea.implicitHeight + header.height + 6, 150) : 0
            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.rowSpan: 2

            color: Style.disabledColor
            radius: 8

            Label {
                id: header
                anchors.top: parent.top
                anchors.topMargin: 4
                anchors.horizontalCenter: parent.horizontalCenter
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 0.9
                text: "Software Update Information"
            }

            ScrollView {
                id: changeLogScrollView

                anchors.top: header.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                TextArea {
                    id: changeLogTextArea
                    text: deviceController.deviceControllerCPP.system.latestVersionChangeLog
                    readOnly: true
                    textFormat: Text.MarkdownText

                    font.family: "Roboto"
                    font.pointSize: Qt.application.font.pointSize * 0.8
                    wrapMode: TextEdit.WordWrap
                    background: Rectangle {
                        color: "transparent"
                    }
                }
            }
        }


    }

    FontMetrics {
        id: fontMetrics
    }

    ItemDelegate {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: root.leftPadding
        anchors.bottomMargin: root.bottomPadding

        rightPadding: 4
        leftPadding: 8

        contentItem: RowLayout {
            RoniaTextIcon {
                id: icon

                Layout.alignment: Qt.AlignLeft
                text: FAIcons.download
            }

            Label {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: root.leftPadding

                color: Style.foreground
                textFormat: Text.MarkdownText
                font.family: "Roboto"
                font.letterSpacing: 1.5
                text: "<u>Download & Install</u>  "
                lineHeight: 0.5
            }
        }

        onClicked: {
            deviceController.deviceControllerCPP.system.partialUpdate();

            // Start download
            uiSession.popupLayout.displayPopUp(uiSession.popUps.installingUpdatePopup)

        }
    }
}
