import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SendingLogPopup to show sending log process.
 * ***********************************************************************************************/

I_PopUp {
    id: root


    /* Property declaration
     * ****************************************************************************************/
    property DeviceController deviceController

    /* Object properties
     * ****************************************************************************************/
    title: ""
    width: AppStyle.size * 0.6
    height: AppStyle.size * 0.30

    /* Children
     * ****************************************************************************************/
    Connections {
        target: deviceController.system

        function onSendLogProgressChanged(percent: int) {
            logProgressStatus.text = "Sending Log..."

            if (percent >= 98)
                percent = 98;

            progressBar.value = percent;
        }

        function onLogSentSuccessfully() {
            progressBar.value = 100;
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
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.75
            text:  "Sending Log..."
            horizontalAlignment: Text.AlignLeft
        }

        //! Sending progress bar
        ProgressBar {
            id: progressBar
            from: 0.0
            to: 100
            value: 0

            Layout.fillWidth: true

            Behavior on value {
                enabled: root.visible

                NumberAnimation  {
                    easing.type: Easing.InOutQuart
                    duration: 500
                }
            }

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
            text: `${progressBar.value.toFixed(0)}% uploaded`
            horizontalAlignment: Text.AlignHCenter
        }

        //! Spacer
        Item {
            Layout.fillWidth: true
            height: 10
        }

    }

    /* Functions
     * ****************************************************************************************/
    //! initialize parameters
    function init() {
        progressBar.value = 0;
        logProgressStatus.text = "Sending Log..."
    }
}
