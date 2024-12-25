import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ResetFactoryPopUp: Displays a confirmation dialog for performing a factory reset, warning the user about data loss.
 * ************************************************************************************************/
I_PopUp {
    id: root


    /* Object Properties
     * ****************************************************************************************/
    height: AppStyle.size * 0.70
    width: AppStyle.size * 0.80
    bottomPadding: 5
    leftPadding: 10
    rightPadding: 5

    title: "You are about to factory reset your device. This action will:"

    /* Signals
     * ****************************************************************************************/
    signal reset()

    /* Children
     * ****************************************************************************************/
    contentItem: ColumnLayout {
        spacing: 16

        Label {
            id: labelTitle

            Layout.fillWidth: true
            Layout.topMargin: 20
            Layout.leftMargin: 24
            Layout.rightMargin: 24

            text: root.title
            visible: text !== ""
            wrapMode: Text.WordWrap
            color: Style.foreground
            horizontalAlignment: Text.AlignHCenter

            font.bold: true
            font.pointSize: 16
        }

        Flickable {
            id: flick

            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollIndicator.vertical: ScrollIndicator {
                parent: flick
                height: parent.height
                x: parent.width
            }

            clip: true
            contentWidth: width
            boundsBehavior: Flickable.StopAtBounds
            contentHeight: messageLabel.implicitHeight

            Label {
                id: messageLabel

                width: parent.width
                textFormat: Text.MarkdownText
                wrapMode: Text.WordWrap
                rightPadding: 4
                text: "1. Remove the device from servers (move to In inventory status).

2. Forget all Wi-Fi profiles.

3. Reset all device preferences and settings to default.

4. Clear all registration data."

                font.pointSize: 12
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16

            ButtonInverted {
                Layout.alignment: Qt.AlignHCenter

                leftPadding: 8
                rightPadding: 8
                text: "Cancel"
                font.pointSize: Qt.application.font.pointSize * 0.8

                onClicked: {
                    root.close()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ButtonInverted {
                Layout.alignment: Qt.AlignHCenter

                leftPadding: 8
                rightPadding: 8
                text: "Reset"
                font.pointSize: Qt.application.font.pointSize * 0.8

                onClicked: {
                    root.reset()
                }
            }
        }
    }
}
