import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * BackdoorPopup: Manage the manual update in test mode.
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property Declaration
     * ****************************************************************************************/

    property System system: deviceController.deviceControllerCPP.system

    /* Object properties
     * ****************************************************************************************/

    title: "Manual System Update"

    /* Children
     * ****************************************************************************************/
    GridLayout {
        id: mainLayout

        property bool isFoundBackdoorFile: false

        anchors.top: parent.top

        height: Math.min(root.availableHeight, implicitHeight)
        width: parent.width * 0.85

        columns: 2
        rowSpacing: 8
        columnSpacing: 32

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize
            text: "Name: "
            lineHeight: 1.5
        }

        TextField {
            id: nameTextField
            Layout.fillWidth: true
            placeholderText: "File name"

            onTextChanged: {
                mainLayout.isFoundBackdoorFile = false;

                // Hide the error label
                errorLabel.visible = false;
            }
        }

        ButtonInverted {

            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            text: "   Find  "

            enabled: nameTextField.text.length > 0

            onClicked: {
                if (system)
                    mainLayout.isFoundBackdoorFile = system.findBackdoorVersion(nameTextField.text);

                errorLabel.visible = !mainLayout.isFoundBackdoorFile;
            }
        }

        Label {
            id: errorLabel

            Layout.columnSpan: 2
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            visible: false;
            font.pointSize: Application.font.pointSize
            color: AppStyle.primaryRed
            text:  nameTextField.text + "\nisn't found on the server, or perhaps \nthe backdoor.json file is damaged."
        }

        Rectangle {

            visible:mainLayout.isFoundBackdoorFile && changeLogTextArea.text.length > 0
            height: changeLogTextArea.text.length > 0 ? Math.min(changeLogTextArea.implicitHeight + header.height + 6, root.height * 0.4 - 10) : 0

            width: root.width * 0.85
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
                    text: system.backdoorLog
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

        Item {
            Layout.columnSpan: 2
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    ItemDelegate {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.bottomPadding

        hoverEnabled: mainLayout.isFoundBackdoorFile
        visible: mainLayout.isFoundBackdoorFile

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
                font.pointSize: Qt.application.font.pointSize  * (mainLayout.isFoundBackdoorFile ? 1.0 : 0.7)
                font.letterSpacing: mainLayout.isFoundBackdoorFile ? 1.5 : 1.0
                text: "<u>Download & Install</u>  "
                lineHeight: 0.5
            }
        }

        onClicked: {
            if (mainLayout.isFoundBackdoorFile && system)
                system.partialUpdate(true);
        }
    }
}
