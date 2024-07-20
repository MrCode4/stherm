import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * privacyPolicyPage.qml privacyPolicy and terms of use
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property bool testMode: false

    property System system: deviceController.deviceControllerCPP.system

    /* Object properties
     * ****************************************************************************************/

    title: "Privacy Policy & Terms Of Use"

    // to fit the title in page
    titleHeadeingLevel: 4

    backButtonVisible: !testMode

    /* Children
     * ****************************************************************************************/

    //! Next button (loads StartTestPage)
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        enabled: privacyPolicyChbox.checked

        onClicked: {
            //! Save accepted version and Load the next page
            if (root.StackView.view) {

                if (testMode) {
                    appModel.userPolicyTerms.acceptedTimeTester = system.getCurrentTime();
                    appModel.userPolicyTerms.acceptedVersionOnTestMode = appModel.userPolicyTerms.currentVersion;
                    root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "backButtonVisible" : false
                                             });

                } else {
                    appModel.userPolicyTerms.acceptedTimeUser = system.getCurrentTime();
                    appModel.userPolicyTerms.acceptedVersion = appModel.userPolicyTerms.currentVersion;
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                             });
                }

                AppCore.defaultRepo.saveToFile(uiSession.configFilePath);
            }
        }
    }

    RowLayout {
        id: confirmRowLayout
        spacing: 4

        anchors.centerIn: parent
        anchors.bottomMargin: 10

        CheckBox {
            id: privacyPolicyChbox
            Layout.leftMargin: -leftPadding
            focusPolicy: Qt.TabFocus

            checked: false
            onClicked: {
                managePrivacyPolicyChbox();
            }
        }

        Label {
            Layout.alignment: Qt.AlignVCenter

            font.pointSize: Application.font.pointSize * 0.85
            textFormat: Text.StyledText
            linkColor: Material.foreground
            verticalAlignment: Text.AlignVCenter
            text: '<p>By checking this box and activating this device,<br> I agree to the <b><a>Privacy Policy</a></b> and <b><a>Terms of use</a></b>,
                   <br>which contain arbitration provisions waiving<br>my right to a jury trial and my right to enforce<br>this contract via class action.</p>'


            TapHandler {
                onTapped: {
                    managePrivacyPolicyChbox();
                }
            }
        }
    }

    //! PrivacyPolicyPopup: To improve memory efficiency, we'll declare this here
    //! as it's also used in the PrivacyPolicyPage.
    PrivacyPolicyPopup {
        id: privacyPolicyPopup
        userPolicyTerms: appModel.userPolicyTerms

    }

    /* Functions
     * ****************************************************************************************/
    function managePrivacyPolicyChbox() {
        privacyPolicyChbox.checked = privacyPolicyPopup.isRead ? !privacyPolicyChbox.checked : false;
        privacyPolicyPopup.open();
    }
}
