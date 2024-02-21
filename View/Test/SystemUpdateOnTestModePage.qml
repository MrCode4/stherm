import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemUpdateOnTestMode show all available versions in the server and allow user install them
 * without any rols.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property System system: deviceController.deviceControllerCPP.system

    property var availableVersions: system?.availableVersions ?? []

    property bool hasAvailableVersions: availableVersions.length > 0

    /* Object properties
     * ****************************************************************************************/
    title: "System Update On Test Mode"

    Component.onCompleted: {
        system.getUpdateInformation();
    }

    /* Children
     * ****************************************************************************************/
    GridLayout {
        id: mainLay
        anchors.fill: parent
        height: Math.min(root.availableHeight, implicitHeight)

        anchors.top: parent.top
        columns: 2
        rowSpacing: 8
        columnSpacing: 32

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Version: ").width + leftPadding + rightPadding
            font.bold: true
            text: "Version: "
        }

        ComboBox {
            id: versionCombobox

            Layout.fillWidth: true
            model: availableVersions
        }

        Rectangle {

            visible: hasAvailableVersions && changeLogTextArea.text.length > 0
            height: changeLogTextArea.text.length > 0 ? Math.min(changeLogTextArea.implicitHeight + header.height + 6, root.height * 0.45) : 0
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
                    text: system?.getLogByVersion(versionCombobox.currentText) ?? ""
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

            hoverEnabled: hasAvailableVersions
            visible: hasAvailableVersions

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
                    font.pointSize: Qt.application.font.pointSize  * (hasAvailableVersions ? 1.0 : 0.7)
                    font.letterSpacing: hasAvailableVersions ? 1.5 : 1.0
                    text: "<u>Download & Install</u>  "
                    lineHeight: 0.5
                }
            }

            onClicked: {
                if (hasAvailableVersions && versionCombobox.currentText.length > 0)
                    system.partialUpdateByVersion(versionCombobox.currentText);
            }
        }

        FontMetrics {
            id: fontMetrics
        }
    }

}
