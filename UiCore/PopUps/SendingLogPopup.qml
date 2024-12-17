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
    title: "Log "
    width: AppStyle.size * 0.85
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

        Label {
            id: logProgressStatus

            Layout.topMargin: 2
            Layout.fillWidth: true

            font.pointSize: Application.font.pointSize
            text: "Sending Log..."
            horizontalAlignment: Text.AlignHCenter
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
