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

        RowLayout {
            width: parent.width

            //! Phone
            Item {
                Layout.preferredWidth: parent.width/3

                RoniaTextIcon {
                    text: FAIcons.circlePhone

                    anchors {
                        centerIn: parent
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: appModel.contactContractor.phoneNumber
            }
        }

        RowLayout {
            width: parent.width

            //! Nuve Url
            Item {
                Layout.preferredWidth: parent.width/3

                RoniaTextIcon {
                    text: FAIcons.globe

                    anchors {
                        centerIn: parent
                    }
                }
            }

            Item {
                Layout.preferredHeight: 130
                Layout.fillWidth: true

                Image {
                    source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg(appModel.contactContractor.qrURL, Style.foreground)}`
                    sourceSize: Qt.size(130, 130)
                }
            }
        }
    }

    ButtonInverted {
        id: requestButton

        text: "Request a Tech"

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
