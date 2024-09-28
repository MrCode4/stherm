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
    property bool manualEntry: appModel?.serviceTitan?.isSTManualMode ?? true


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
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95

        Label {
            visible: fullNameLabel.visible
            text: "Full Name"
            font.pointSize: root.font.pointSize * 0.9
        }

        Label {
            id: fullNameLabel

            visible: !manualEntry
            text: appModel?.serviceTitan?.fullName ?? ""
            font.pointSize: root.font.pointSize * 0.8
        }


        Item {
            Layout.fillWidth: true
            height: emailTf.implicitHeight + 5

            Label {
                id: emailLabel
                anchors.top: parent.top
                text: "Email"
                font.pointSize: root.font.pointSize * 0.9
            }

            TextField {
                id: emailTf
                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.right: parent.right
                anchors.left: parent.left

                topPadding: 0
                placeholderText: "Input the Email"
                font.pointSize: root.font.pointSize * 0.8
                text: appModel?.serviceTitan?.email ?? ""
                validator: RegularExpressionValidator {
                    regularExpression: /^[^\s@]+@[^\s@]+\.[^\s@]+$/
                }
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


        Item {
            Layout.fillWidth: true
            height: zipCodeTf.implicitHeight + 5

            Label {
                anchors.top: parent.top
                text: "ZIP code"
                font.pointSize: root.font.pointSize * 0.9
            }


            TextField {
                id: zipCodeTf

                anchors.top: parent.top
                anchors.topMargin: 5
                anchors.right: parent.right
                anchors.left: parent.left

                placeholderText: "Input the ZIP code"
                text: appModel?.serviceTitan?.zipCode ?? ""
                font.pointSize: root.font.pointSize * 0.8
                validator: RegularExpressionValidator {
                    regularExpression: /^\d{5}(-\d{4})?$/
                }

                inputMethodHints: Qt.ImhPreferNumbers
            }
        }

        Item {
            Layout.fillWidth: true
            height: 20
        }


        Text {
            id: warrantyReplacementText

            visible: manualEntry
            text: qsTr("Warranty Replacement")
            font.underline: true
            color: "#43E0F8"

            TapHandler {
                onTapped: {
                    // Go to the warranty page
                    if (root.StackView.view) {
                        root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/WarrantyReplacementPage.qml", {
                                                     "uiSession": uiSession
                                                 });
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
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

            nextPage();
        }
    }

    /* Functions
     * ****************************************************************************************/

    //! Go to CustomerDetailsPage
    function nextPage() {
        // Go to next page
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/ServiceTitanReviewPage.qml", {
                                         "uiSession": uiSession,
                                         "initialSetup": root.initialSetup
                                     });
        }
    }
}
