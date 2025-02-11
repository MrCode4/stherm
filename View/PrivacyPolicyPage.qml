import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * PrivacyPolicyPage: privacyPolicy and terms of use
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property bool testMode: false

    property System system: deviceController.deviceControllerCPP.system

    property bool openFromNoWiFiInstallation: false

    property bool _canGoToNextPage: true

    /* Object properties
     * ****************************************************************************************/

    title: "Privacy Policy & Terms Of Use"

    // to fit the title in page
    titleHeadeingLevel: 4

    backButtonVisible: !testMode

    onVisibleChanged: {
        if (visible)
            root._canGoToNextPage = true;
    }

    /* Children
     * ****************************************************************************************/

    CheckBox {
        id: privacyPolicyChbox

        anchors.left: parent.left
        anchors.leftMargin: leftPadding
        anchors.verticalCenter: parent.verticalCenter

        checked: false

        indicator.width: 30
        indicator.height: 30

        // Use TapHandler to handle checked in the checkbox.
        TapHandler {
            onTapped: {
                if (privacyPolicyPopup.isRead) {
                    privacyPolicyChbox.checked = !privacyPolicyChbox.checked;
                    privacyPolicyPopup.isAccepted = privacyPolicyChbox.checked;

                } else {
                    managePrivacyPolicyChbox();
                }
            }
        }
    }

    Label {
        anchors.left: privacyPolicyChbox.right
        anchors.right: parent.right
        anchors.leftMargin: leftPadding
        anchors.verticalCenter: parent.verticalCenter

        font.pointSize: Application.font.pointSize * 0.85
        textFormat: Text.StyledText
        linkColor: Material.foreground
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        text:  '<p>By checking this box and activating this device, I agree to the <b><a>Privacy Policy</a></b> and <b><a>Terms of use</a></b>,
                    which contain arbitration provisions waiving my right to a jury trial and my right to enforce this contract via class action.</p>'

        TapHandler {
            onTapped: {
                managePrivacyPolicyChbox();
            }
        }
    }

    //! PrivacyPolicyPopup: To improve memory efficiency, we declared this here
    PrivacyPolicyPopup {
        id: privacyPolicyPopup
        userPolicyTerms: appModel.userPolicyTerms

        onOpened: {
            if (!isRead)
                uiSession.toastManager.showToast("You need to scroll to the end to accept.", "");
        }

        onClosed: {
            if (isAccepted) {
                privacyPolicyChbox.checked = isRead;
                nextPageTimer.start();
            }
        }
    }

    Timer {
        id: nextPageTimer

        interval: 100
        repeat: false
        running: false

        onTriggered: {
            nextPage();
        }
    }

    /* Functions
     * ****************************************************************************************/
    function managePrivacyPolicyChbox() {
        privacyPolicyPopup.open();
    }

    function nextPage() {
        //! Save accepted version and Load the next page
        if (root._canGoToNextPage && root.StackView.view) {
            root._canGoToNextPage = false;

            if (testMode) {
                appModel.userPolicyTerms.acceptedTimeTester = system.getCurrentTime();
                appModel.userPolicyTerms.acceptedVersionOnTestMode = appModel.userPolicyTerms.currentVersion;
                root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                             "uiSession": Qt.binding(() => uiSession),
                                             "backButtonVisible" : false
                                         });

            } else if (openFromNoWiFiInstallation) {
                root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/CustomerDetailsPage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup,
                                             "openFromNoWiFiInstallation": true
                                         });
            } else {
                appModel.userPolicyTerms.acceptedTimeUser = system.getCurrentTime();
                appModel.userPolicyTerms.acceptedVersion = appModel.userPolicyTerms.currentVersion;
                root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                         });
            }

            deviceController.saveSettings();


        }
    }
}
