import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * CustomerDetailsPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false


    /* Object properties
     * ****************************************************************************************/
    title: "Customer Details"

    /* Children
     * ****************************************************************************************/
    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                             "uiSession": Qt.binding(() => uiSession)
                                         });
            }
        }
    }

    ColumnLayout {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95

        Label {
            visible: fullNameLabel.visible
            text: "Full Name"
            font.pointSize: root.font.pointSize
        }

        Label {
            id: fullNameLabel

            visible: appModel?.serviceTitan?.isSTManualMode ?? false
            text: appModel?.serviceTitan?.fullName ?? ""
            font.pointSize: root.font.pointSize * 0.9
        }

        Label {
            text: "Email"
            font.pointSize: root.font.pointSize
        }

        TextField {
            id: emailTf

            topPadding: 0
            Layout.fillWidth: true
            placeholderText: "Email"
            text: appModel?.serviceTitan?.email ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s@]+@[^\s@]+\.[^\s@]+$/
            }
        }

        Label {
            Layout.fillWidth: true

            width: parent.width
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.67
            text: "Email will be used as a Mobile App login credential."
        }

        Item {
            id: spacer
            Layout.fillWidth: true
            height: 20
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize
        }

        TextField {
            id: zipCodeTf

            Layout.fillWidth: true

            topPadding: 0
            placeholderText: "ZIP code"
            text: appModel?.serviceTitan?.zipCode ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^\d{5}(-\d{4})?$/
            }
        }


        Item {
            Layout.fillWidth: true
            height: root.height / 6
        }

        Text {
            id: warrantyReplacementText

            visible: fullNameLabel.visible
            text: qsTr("Warranty Replacement")
            font.underline: true
            color: "#43E0F8"

            TapHandler {
                onTapped: {
                    // TODO: Go to the warranty page

                }
            }
        }

    }

    //! Next button
    ButtonInverted {
        id: nextBtn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        text: "Next"
        visible: !emailTf.activeFocus && !zipCodeTf.activeFocus
        enabled: emailTf.acceptableInput && zipCodeTf.acceptableInput
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            appModel.serviceTitan.email   = emailTf.text;
            appModel.serviceTitan.zipCode = zipCodeTf.text;

            // Go to next page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/ServiceTitanReviewPage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                         });
            }
        }
    }

}
