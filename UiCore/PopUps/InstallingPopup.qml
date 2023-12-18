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

    width: Application.width * 0.8
    height: Application.height * 0.8

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
            text: "Installing Update..."
            horizontalAlignment: Text.AlignLeft
        }

        //! Download progress bar
        ProgressBar {
            id: progressBar
            from: 0.0
            to: 100
            value: 100

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
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.75
            text: "About 2 mins"
            horizontalAlignment: Text.AlignLeft
        }
    }
}
