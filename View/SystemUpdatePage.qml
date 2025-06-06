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

    property string installedVesion: ""

    property bool mandatoryUpdate: deviceController.mandatoryUpdate;

    property bool contractorFlow: false

    /* Object properties
     * ****************************************************************************************/
    title: "System Update"

    backButtonVisible: !mandatoryUpdate

    //! Send request to get update information from server
    Component.onCompleted: {
        const versionArray = Application.version.split('.')
        const versionArrayMain = versionArray.splice(0, 3)
        installedVesion = versionArrayMain.join('.')
        system.fetchUpdateInformation();
    }

    /* Children
     * ****************************************************************************************/
    //! Next/Confirm button
    ToolButton {
        parent: root.header.contentItem
        enabled: visible && !mandatoryUpdate
        visible: contractorFlow

        RoniaTextIcon {
            anchors.centerIn: parent
            text: FAIcons.arrowRight
        }

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/ContractorInformationPage.qml", {
                                             "uiSession": uiSession
                                         });
            }
        }

    }

    GridLayout {
        height: Math.min(root.availableHeight, implicitHeight)

        anchors.top: parent.top
        columns: 2
        rowSpacing: 8
        columnSpacing: 32

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Update release date: ").width + leftPadding + rightPadding
            font.bold: true
            text: system.updateAvailable ? "Update Release date: " : "Last Update date: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignLeft
            text: system.updateAvailable ? system.latestVersionDate : system.lastInstalledUpdateDate
        }

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Update release date: ").width + leftPadding + rightPadding
            font.bold: true
            text: "Installed Version: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignLeft
            text: installedVesion
        }

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Update release date: ").width + leftPadding + rightPadding
            font.bold: true
            text: system.updateAvailable ? "Update Available: " : "Latest Version: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignLeft
            text: system.latestVersion
        }

        Rectangle {

            visible: system.updateAvailable && changeLogTextArea.text.length > 0
            height: changeLogTextArea.text.length > 0 ? Math.min(changeLogTextArea.implicitHeight + header.height + 6, root.height * 0.45) -
                                                        (manualLayout.visible ? manualLayout.height : 0) : 0
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

                // To inactive horizontal scroll and its bound animations
                contentWidth: availableWidth

                //! to show scroll if needed on show
                ScrollBar.vertical.interactive: false
                ScrollBar.vertical.active: true
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.horizontal.active: false
                ScrollBar.horizontal.interactive: false

                TextArea {
                    id: changeLogTextArea
                    text: system.latestVersionChangeLog
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

        ItemDelegate {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            hoverEnabled: system.updateAvailable
            visible: system.updateAvailable

            rightPadding: 4
            leftPadding: 8

            contentItem: RowLayout {
                RoniaTextIcon {
                    Layout.alignment: Qt.AlignLeft
                    text:  FAIcons.download
                }

                Label {
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: root.leftPadding

                    color: Style.foreground
                    textFormat: Text.MarkdownText
                    font.family: "Roboto"
                    font.pointSize: Qt.application.font.pointSize  * (system.updateAvailable ? 1.0 : 0.7)
                    font.letterSpacing: system.updateAvailable ? 1.5 : 1.0
                    text: "<u>Download & Install</u>  "
                    lineHeight: 0.5
                }
            }

            onClicked: {
                if (system.updateAvailable)
                    system.partialUpdate();
            }
        }
    }

    ItemDelegate {
        anchors.bottom: manualLayout.visible ? manualLayout.top : parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: root.leftPadding
        anchors.bottomMargin: root.bottomPadding

        hoverEnabled: false
        visible: !system.updateAvailable

        rightPadding: 4
        leftPadding: 8

        contentItem: RowLayout {
            RoniaTextIcon {
                Layout.alignment: Qt.AlignLeft
                text: FAIcons.circleInfo
            }

            Label {
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: root.leftPadding

                color: Style.foreground
                textFormat: Text.MarkdownText
                font.family: "Roboto"
                font.pointSize: Qt.application.font.pointSize  * (system.updateAvailable ? 1.0 : 0.7)
                font.letterSpacing: system.updateAvailable ? 1.5 : 1.0
                text: "This version is up to date."
                lineHeight: 0.5
            }
        }
    }

    RowLayout {
        id: manualLayout

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottomMargin: root.bottomPadding

        visible: system.testMode || system.isManualUpdate

        ButtonInverted {
            id: manualUpdateBtn

            Layout.alignment: exitManualUpdateBtn.visible ? Qt.AlignLeft : Qt.AlignHCenter

            visible: system.testMode
            leftPadding: 8
            rightPadding: 8
            text:"Manual Update"

            onClicked: {
                if (system) {
                    system.fetchBackdoorInformation();
                    if (root.StackView.view) {
                        root.StackView.view.push("qrc:/Stherm/View/BackdoorUpdatePage.qml", {
                                                     "uiSession": root.uiSession
                                                 });
                    }
                }
            }
        }

        ButtonInverted {
            id: exitManualUpdateBtn

            Layout.alignment: manualUpdateBtn.visible ? Qt.AlignRight : Qt.AlignHCenter

            visible: system?.isManualUpdate ?? false
            leftPadding: 8
            rightPadding: 8
            text: manualUpdateBtn.visible ? "Exit manual\n mode" : "Exit manual mode"

            onClicked: {
                // Exit from manual mode
                if (system) {
                    system.exitManualMode();
                }
            }
        }
    }

    FontMetrics {
        id: fontMetrics
    }
}
