import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ContactContractorPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: ""

    /* Children
     * ****************************************************************************************/
    GridLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        columns: 2
        rowSpacing: 16
        columnSpacing: 32

        NexgenIcon {
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: root.availableWidth * 0.5
            Layout.preferredHeight: sourceSize.height * width / sourceSize.width + 16
            Layout.bottomMargin: 48
        }

        //! Phone
        RoniaTextIcon {
            text: FAIcons.circlePhone
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "7144717965"
        }

        //! Nuve Url
        RoniaTextIcon {
            text: FAIcons.globe
        }

        Image {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.preferredHeight: width
            Layout.leftMargin: 32
            Layout.rightMargin: 32

            source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg("https://www.nuvehome.com/", Style.foreground)}`
            sourceSize.width: width
            sourceSize.height: height
        }

        //! Request a job
        RoniaTextIcon {
            text: FAIcons.briefcase
        }

        ButtonInverted {
            Layout.alignment: Qt.AlignCenter
            font.bold: true
            text: "Request a Job"
        }
    }
}
