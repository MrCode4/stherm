import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InstallingPopup to show installing update process (download progress).
 * ***********************************************************************************************/

I_PopUp {

    /* Object properties
     * ****************************************************************************************/
    title: ""

    /* Property Declaration
     * ****************************************************************************************/

    property DeviceController deviceController

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.75
            text: "Downloading Update..."
            horizontalAlignment: Text.AlignLeft
        }

        //! Download progress bar
        ProgressBar {
            Layout.preferredWidth: fontMetric.advanceWidth(" About     seconds remaining ") + 6

            id: progressBar
            from: 0.0
            to: 100
            value: deviceController.deviceControllerCPP.system.partialUpdateProgress

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

        Label {
            id: remainingLabel

            Layout.fillWidth: true
            Layout.preferredWidth: fontMetric.advanceWidth(" About     seconds remaining ") + 6

            font.pointSize: Application.font.pointSize * 0.75
            text: deviceController.deviceControllerCPP.system.remainingDownloadTime
            horizontalAlignment: Text.AlignLeft

        }

        FontMetrics {
            id: fontMetric
            font.pointSize: remainingLabel.font.pointSize
        }

        Item {
            id: spacer2

            Layout.fillWidth: true
            height: 10

        }
    }

}
