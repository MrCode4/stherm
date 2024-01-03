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

    property System system: deviceController.deviceControllerCPP.system

    /* Object properties
     * ****************************************************************************************/
    title: "System Update"

    //! Send request to get update information from server
    Component.onCompleted: {
        system.getUpdateInformation();
    }

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

                //! to show scroll if needed on show
                ScrollBar.vertical.interactive: false
                ScrollBar.vertical.active: true
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

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

        hoverEnabled: system.updateAvailable

        rightPadding: 4
        leftPadding: 8

        contentItem: RowLayout {
            RoniaTextIcon {
                id: icon

                Layout.alignment: Qt.AlignLeft
                text: system.updateAvailable ? FAIcons.download : FAIcons.circleInfo
            }

            Label {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: root.leftPadding

                color: Style.foreground
                textFormat: Text.MarkdownText
                font.family: "Roboto"
                font.pointSize: Qt.application.font.pointSize  * (system.updateAvailable ? 1.0 : 0.7)
                font.letterSpacing: system.updateAvailable ? 1.5 : 1.0
                text: system.updateAvailable ? "<u>Download & Install</u>  " : "This version is up to date."
                lineHeight: 0.5
            }
        }

        onClicked: {
            if (system.updateAvailable)
                system.partialUpdate();
        }
    }
}
