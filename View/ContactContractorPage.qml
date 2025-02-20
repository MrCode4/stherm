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
        id: organizationIcon
        appModel: root.appModel

        width: parent.width * 0.5
        height: parent.height * 0.25

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
    }

    Column {
        width: parent.width
        spacing: 24

        anchors {
            top: organizationIcon.bottom
            topMargin: 40
            bottom: requestButton.top
        }

        Item {
            width: parent.width
            height: phoneNumberText.height

            //! Phone
            RoniaTextIcon {
                text: FAIcons.circlePhone

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    horizontalCenterOffset: -125
                    verticalCenter: phoneNumberText.verticalCenter
                }
            }

            Label {
                id: phoneNumberText

                text: appModel.contactContractor.phoneNumber

                anchors {
                    centerIn: parent
                }
            }
        }

        Item {
            width: parent.width
            height: qrCodeImage.height

            //! Nuve Url
            RoniaTextIcon {
                text: FAIcons.globe

                anchors {
                    centerIn: parent
                    horizontalCenterOffset: -125
                }
            }

            Image {
                id: qrCodeImage

                source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg(appModel.contactContractor.qrURL, Style.foreground)}`
                sourceSize: Qt.size(130, 130)

                anchors {
                    centerIn: parent
                }
            }
        }
    }

    ButtonInverted {
        id: requestButton

        text: "Request a Tech"
        visible: false

        font {
            bold: true
        }

        anchors {
            bottom: parent.bottom
            bottomMargin: 20
            horizontalCenter: parent.horizontalCenter
        }

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/RequestTechPriorityPage.qml", {
                                             "uiSession": uiSession
                                         });
            }
        }
    }
}
