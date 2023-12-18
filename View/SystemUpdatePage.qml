import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemUpdatePage retrieves system update information and prepare device to update.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property delcaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Update"

    /* Children
     * ****************************************************************************************/
    GridLayout {
        height: Math.min(root.availableHeight, implicitHeight)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        columns: 2
        rowSpacing: 16
        columnSpacing: 32

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Last update date: ").width + leftPadding + rightPadding
            font.bold: true
            text: "Last update date: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignRight
            text: ""
        }

        Label {
            Layout.preferredWidth: fontMetrics.boundingRect("Last update date: ").width + leftPadding + rightPadding
            font.bold: true
            text: "Update Available: "
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.9
            horizontalAlignment: Text.AlignRight
            text: ""
        }

    }

    FontMetrics {
        id: fontMetrics
    }

}
