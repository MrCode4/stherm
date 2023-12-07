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
        height: Math.min(root.availableHeight, implicitHeight)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        columns: 2
        rowSpacing: 16
        columnSpacing: 32

        OrganizationIcon {
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 48
            Layout.preferredHeight: root.height / 8
            Layout.fillWidth: true
            Layout.preferredWidth: 0
        }

        //! Phone
        RoniaTextIcon {
            text: FAIcons.circlePhone
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: "(714)-4717965"
        }

        //! Nuve Url
        RoniaTextIcon {
            text: FAIcons.globe
        }

        Image {
            Layout.alignment: Qt.AlignCenter

            source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg("https://www.nuvehome.com/", Style.foreground)}`
            sourceSize.height: 130
            sourceSize.width: 130
        }

        //! Request a job
        RoniaTextIcon {
            text: FAIcons.briefcase
        }

        ButtonInverted {
            id: requestJobBtn
            Layout.alignment: Qt.AlignCenter
            font.bold: true
            text: "Request a Job"
        }
    }
}
