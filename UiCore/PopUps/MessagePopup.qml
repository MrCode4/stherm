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
    keepOpen: true

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
            id: labelTitle
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            Layout.topMargin: -5
            Layout.leftMargin: 0

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
            clip: true

            Component.onCompleted: {
                titleAnimationTimer.needed = labelTitle.contentWidth > labelTitle.width + 10
            }

            Timer {
                id: titleAnimationTimer
                running: labelTitle.visible && needed
                interval: 50
                repeat: true

                property int endAnimationDelay: 10
                property bool needed : false

                onTriggered: {
                   labelTitle.leftPadding = labelTitle.leftPadding - 1;
                    if (labelTitle.contentWidth + labelTitle.leftPadding  < labelTitle.width + 1)
                        endAnimationDelay--;

                    if (endAnimationDelay == 0){
                        endAnimationDelay = 10;
                        labelTitle.leftPadding = 10;
                    }
                }
            }
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

                property string processedMessage: message?.message ?? ""

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
