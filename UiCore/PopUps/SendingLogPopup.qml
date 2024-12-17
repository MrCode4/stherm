import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SendingLogPopup to show sending log process.
 * ***********************************************************************************************/

I_PopUp {

    /* Object properties
     * ****************************************************************************************/
    title: "Log Status"
    width: AppStyle.size * 0.8
    height: AppStyle.size * 0.50

    /* Children
     * ****************************************************************************************/
    Connections {
        target: deviceController.system

        function onSendLogProgressChanged(percent: int) {
            logProgressStatus.text = "Sending Log..."
            progressBar.value = percent;
        }

        function onLogSentSuccessfully() {
            logProgressStatus.text = "Log is sent!";
        }
    }

    ColumnLayout {
        id: mainLay

        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 5

            //! Icon
            RoniaTextIcon {
                Layout.alignment: Qt.AlignLeft

                font.pointSize: Qt.application.font.pointSize * 2.4
                text: FAIcons.circleInfo
                font.weight: FAIcons.Light
            }

            Label {
                id: logProgressStatus

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                Layout.fillWidth: true

                font.pointSize: Application.font.pointSize
                text: "Sending Log..."
                horizontalAlignment: Text.AlignHCenter
            }
        }

        //! Spacer
        Item {
            Layout.fillWidth: true
            height: 10
        }

        //! Sending progress bar
        ProgressBar {
            Layout.fillWidth: true

            id: progressBar
            from: 0.0
            to: 100
            value: 0

            contentItem: Item {
                Rectangle {
                    width: progressBar.visualPosition * parent.width
                    height: parent.height
                    radius: 2
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#80589F"}
                        GradientStop { position: 1.0; color: "#9BD1F7" }
                    }
                }
            }
        }

        //! Progress Value Label
        Label {
            id: progressLabel

            Layout.fillWidth: true

            font.pointSize: Application.font.pointSize * 0.75
            text: progressBar.value + "%"
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
