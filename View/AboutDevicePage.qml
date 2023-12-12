import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AboutDevicePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Device Info"

    /* Childrent
     * ****************************************************************************************/
    ListView {
        id: _infoLv

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: _root.contentItem.y
            parent: _root
            height: _root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        clip: true
        model: [
            { "key": "Model",               "value": "Nuve - Samo" },
            { "key": "FCC ID",              "value": "2BBXVSAMOV1" },
            { "key": "Contians FCC ID",     "value": "VPYLB1DX" },
            { "key": "IC",                  "value": "LBWA1KL1FX-875" },
            { "key": "Serial No",           "value": "01323000001" },
            { "key": "Custom Name",         "value": "Living Room" },
            { "key": "URL",                 "value": '<a href="nuvehome.com" style="text-decoration:none;color:#44A0FF;">nuvehome.com</a>' },
            { "key": "E-mail",              "value": '<a href="support@nuvehome.com" style="text-decoration:none;color:#44A0FF;">support@nuvehome.com</link>' },
            { "key": "Software version",    "value": Application.version },
            { "key": "Hardware version",    "value": "01" },
        ]
        delegate: RowLayout {
            width: ListView.view.width
            height: Style.delegateHeight * 0.8
            spacing: 16

            Label {
                Layout.preferredWidth: _fontMetrics.boundingRect("Hardware version :").width + leftPadding + rightPadding
                font.bold: true
                text: modelData.key + ":"
            }

            Label {
                Layout.fillWidth: true
                font.pointSize: Application.font.pointSize * 0.9
                textFormat: "RichText"
                horizontalAlignment: "AlignRight"
                text: modelData.value
            }
        }
    }

    FontMetrics {
        id: _fontMetrics
    }
}
