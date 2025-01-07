import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ManualDateTimeWarningPopup: Displays a warning that the device time may be incorrect after a power loss.
 * ************************************************************************************************/
I_PopUp {
    id: root

    /* Object Properties
     * ****************************************************************************************/
    height: AppStyle.size * 0.72
    width: AppStyle.size * 0.80
    bottomPadding: 5
    leftPadding: 10
    rightPadding: 5

    title: qsTr("Device Time May Be Incorrect After Power Loss")

    /* Signals
     * ****************************************************************************************/
    signal accepted()

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

        Label {
            id: labelTime

            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24

            text: DateTimeManager.now.toLocaleString()
            visible: text !== ""
            wrapMode: Text.WordWrap
            color: Style.foreground
            horizontalAlignment: Text.AlignHCenter

            font.bold: true
            font.pointSize: 14
        }

        Label {
            id: labelInfo

            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24

            text: qsTr("If the time is inaccurate, you may need to adjust it manually or enable automatic time synchronization for accuracy.")

            visible: text !== ""
            wrapMode: Text.WordWrap
            color: Style.foreground
            horizontalAlignment: Text.AlignHCenter

            font.bold: false
            font.pointSize: 14
        }


        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 20
            spacing: 24

            ButtonInverted {

                leftPadding: 8
                rightPadding: 8
                text: qsTr("Ok")

                onClicked: {
                    root.close();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ButtonInverted {

                leftPadding: 8
                rightPadding: 8
                text: qsTr("Change")

                onClicked: {
                    root.accepted();
                }
            }
        }
    }
}
