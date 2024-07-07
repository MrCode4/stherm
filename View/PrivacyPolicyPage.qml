import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * privacyPolicy and terms of use
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
                    appModel.userPolicyTerms.acceptedVersionOnTestMode = appModel.userPolicyTerms.currentVersion;
                    root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "backButtonVisible" : false
                                             });

                } else {
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


    Flickable {
        id: privacyFlick

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: confirmRowLayout.top
        anchors.bottomMargin: 10

        property bool isRead: false

        ScrollIndicator.vertical: ScrollIndicator {
            parent: privacyFlick.parent
            height: parent.height
            x: parent.width

            onPositionChanged: {
                if (!privacyFlick.isRead)
                    privacyFlick.isRead = position > 0.98;
            }
        }

        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: privacyPolicyLabel.implicitHeight

        Label {
            id: privacyPolicyLabel
            anchors.fill: parent

            text: "## Privacy Policy V. " + appModel.userPolicyTerms.currentVersion + "\n\n" +
                  appModel.userPolicyTerms.privacyPolicy + "\n\n\n\n" +
                  "## Terms Of Use V. " + appModel.userPolicyTerms.currentVersion + "\n\n" +
                  appModel.userPolicyTerms.termsOfUse

            leftPadding: 4;
            rightPadding: 4
            background: null
            textFormat: Text.MarkdownText
            wrapMode: Text.WordWrap
            lineHeight: 1.3
            font.pointSize: Qt.application.font.pointSize * 0.7
        }
    }

    RowLayout {
        id: confirmRowLayout
        spacing: 4

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        enabled: privacyPolicyLabel.text.length > 0 && privacyFlick.isRead

        CheckBox {
            id: privacyPolicyChbox
            Layout.leftMargin: -leftPadding
            focusPolicy: Qt.TabFocus

            checked: false
        }

        Label {
            Layout.alignment: Qt.AlignVCenter

            font.pointSize: Application.font.pointSize * 0.85
            textFormat: Text.StyledText
            linkColor: Material.foreground
            verticalAlignment: Text.AlignVCenter
            text: '<p>I agree to <b><a>Privacy Policy</a></b> and<br><b><a>Terms of use</a></b>.</p>'

            TapHandler {
                onTapped: {
                    privacyPolicyChbox.checked = !privacyPolicyChbox.checked;
                }
            }
        }
    }
}
