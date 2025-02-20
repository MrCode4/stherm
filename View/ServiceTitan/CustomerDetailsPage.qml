import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * CustomerDetailsPage
 * ***********************************************************************************************/
InitialSetupBasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool manualEntry: appModel?.serviceTitan?.isSTManualMode ?? true

    property bool openFromNoWiFiInstallation: false

    /* Object properties
     * ****************************************************************************************/
    title: "Customer Details"

    /* Children
     * ****************************************************************************************/
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


        GridLayout {
            Layout.fillWidth: true
            height: zipCodeTf.implicitHeight + 5

            columns: 2
            columnSpacing: 15
            rowSpacing: 0

            Label {
                text: "Country"
                font.pointSize: root.font.pointSize * 0.9
            }

            Label {
                text: "ZIP code"
                font.pointSize: root.font.pointSize * 0.9
            }

            ComboBox {
                id: countryCombobox

                Layout.preferredWidth: root.availableWidth / 2 - 10
                Layout.alignment: Qt.AlignBottom

                font.pointSize: root.font.pointSize * 0.8
                model: AppSpec.supportedCountries
                currentIndex: appModel?.serviceTitan?.country.length > 0 ?
                                  AppSpec.supportedCountries.indexOf(appModel?.serviceTitan?.country) : 0
            }

            TextField {
                id: zipCodeTf

                Layout.preferredWidth: root.availableWidth  / 2 - 10

                placeholderText: "Input the ZIP code"
                text: appModel?.serviceTitan?.zipCode.toUpperCase() ?? ""
                font.pointSize: root.font.pointSize * 0.8

                // Australia: 4 digits
                // Canada: 6 letters + digits
                // US: 5 digits: 10498
                validator: RegularExpressionValidator {
                    regularExpression: /^(?:\d{4,5}|[A-Z\d]{6})$/i
                }

                inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhPreferUppercase
            }
        }

        Item {
            Layout.fillWidth: true
            height: 20
        }


        Text {
            id: warrantyReplacementText

            visible: manualEntry && !openFromNoWiFiInstallation
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
        visible: !openFromNoWiFiInstallation && !emailTf.activeFocus && !zipCodeTf.activeFocus
        enabled: emailTf.acceptableInput && zipCodeTf.acceptableInput
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            updateModel();
            nextPage();
        }
    }

    BusyIndicator {
        anchors.bottom: submitBtn.top
        anchors.horizontalCenter: submitBtn.horizontalCenter
        anchors.margins: 10

        height: 45
        width: 45
        visible: deviceController.isSendingInitialSetupData && submitBtn.visible
        running: visible
    }


    //! Submit button
    ButtonInverted {
        id: submitBtn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: 50
        anchors.bottomMargin: 10

        enabled: !deviceController.isSendingInitialSetupData && emailTf.acceptableInput && zipCodeTf.acceptableInput
        text: "Submit"
        visible: openFromNoWiFiInstallation && !emailTf.activeFocus && !zipCodeTf.activeFocus
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            updateModel();
            deviceController.pushInitialSetupInformation();
        }
    }

    /* Functions
     * ****************************************************************************************/

    function updateModel() {
        appModel.serviceTitan.email   = emailTf.text;
        appModel.serviceTitan.zipCode = zipCodeTf.text;
        appModel.serviceTitan.country = countryCombobox.currentText;
    }

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
