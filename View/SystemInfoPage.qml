import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemInfoPage retrieves device info and displays them
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property delcaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Info"

    /* Children
     * ****************************************************************************************/
    ListView {
        anchors.fill: parent
        clip: true
        spacing: 12
        model: [
            "OS",
            "Kernel",
            "Kernel Version",
            "DPR",
            "L-DPI",
            "P-DPI",
            "Width",
            "Height",
            "Nmcli"
        ]
        delegate: Control {
            id: _infoDel
            width: ListView.view.width
            height: Material.delegateHeight

            ColumnLayout {
                anchors.fill: parent

                Label {
                    opacity: 0.8
                    font.pointSize: _infoDel.font.pointSize * 0.65
                    text: modelData
                }

                Label {
                    Layout.fillHeight: true
                    Layout.leftMargin: 12
                    verticalAlignment: Text.AlignVCenter
                    text: deviceInfo[modelData]
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Material.frameColor
                }
            }
        }
    }
}
