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
    OrganizationIcon {
        appModel: root.appModel

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.5
        height: parent.height * 0.25
    }

    GridLayout {
        height: Math.min(root.availableHeight, implicitHeight)
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        columns: 2
        rowSpacing: 16
        columnSpacing: 32

        Item {

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
            text: appModel.contactContractor.phoneNumber
        }

        //! Nuve Url
        RoniaTextIcon {
            text: FAIcons.globe
        }

        Image {
            Layout.alignment: Qt.AlignCenter

            source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg(appModel.contactContractor.qrURL, Style.foreground)}`
            sourceSize.height: 130
            sourceSize.width: 130
        }

        //! Request a Tech
        RoniaTextIcon {
            text: FAIcons.briefcase
        }

        ButtonInverted {
            Layout.alignment: Qt.AlignCenter
            font.bold: true
            text: "Request a Tech"

            onClicked: {
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/RequestTechPriorityPage.qml", {
                                                  "uiSession": uiSession
                                              });
                }
            }
        }
    }
}
