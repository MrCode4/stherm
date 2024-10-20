import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import Ronia
import Stherm

/*! ***********************************************************************************************
 * MessagePopup: Shows the server messages.
 * ************************************************************************************************/
I_PopUp {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Popup Message
    property Message    message

    /* Object Properties
     * ****************************************************************************************/

    height: Math.min(AppStyle.size * 0.80, flick.contentHeight + headerLayout.height + 100)
    width: AppStyle.size * 0.80
    bottomPadding: 5
    leftPadding: 10
    rightPadding: 5

    closePolicy: Popup.NoAutoClose

    /* Children
     * ****************************************************************************************/

    contentItem: ColumnLayout {
        //! Header
        RowLayout {
            id: headerLayout
            property int labelMargin: 0

            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillHeight: false

            spacing: 16

            //! Icon
            RoniaTextIcon {
                font.pointSize: Qt.application.font.pointSize * 1.2
                text: FAIcons.envelope
                font.weight: 300
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: parent.labelMargin
                horizontalAlignment: Text.AlignLeft
                textFormat: Text.MarkdownText
                text: "### Message"
                elide: Text.ElideRight
                color: enabled ? Style.foreground : Style.hintTextColor
                linkColor: Style.linkColor
            }

            ToolButton {
                Layout.rightMargin: - root.rightPadding + 4
                visible: closeButtonEnabled
                contentItem: RoniaTextIcon {
                    font.pointSize: Application.font.pointSize * 1.2
                    text: FAIcons.xmark
                }

                onClicked: {
                    root.close();
                }
            }
        }

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            Layout.topMargin: -5
            Layout.leftMargin: parent.labelMargin

            property string processedTitle: message?.title ?? ""

            visible: processedTitle !== ""
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.MarkdownText
            text: `${"#".repeat(5)} ${processedTitle}`
            font.bold: true
            elide: Text.ElideRight
            font.pointSize: Application.font.pointSize * 1.1
            color: enabled ? Style.foreground : Style.hintTextColor
            linkColor: Style.linkColor
        }

        Flickable {
            id: flick

            Layout.fillHeight: true
            Layout.fillWidth: true

            ScrollIndicator.vertical: ScrollIndicator {
                parent: flick
                height: parent.height
                x: parent.width
            }

            clip: true
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: width
            contentHeight: messageLabel.implicitHeight

            Label {
                id: messageLabel

                //! The device is unable to recognize the asterisk followed by a space ("* ") as a valid bullet point. Therefore,
                //! we have implemented a workaround to replace it with a hyphen followed by a space ("- ").
                property string processedMessage: message?.message?.replace(/\* /g, '- ') ?? ""

                anchors.fill: parent

                text: processedMessage
                leftPadding: 4;
                rightPadding: 4
                background: null
                textFormat: Text.MarkdownText
                wrapMode: Text.WordWrap
                lineHeight: 1.3
                font.pointSize: Qt.application.font.pointSize * 0.8
            }

        }
    }
}
