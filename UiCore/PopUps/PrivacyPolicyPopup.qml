import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * PrivacyPolicyPopup: Shows the privacy policy and terms of use
 * ***********************************************************************************************/

I_PopUp {
    id: root

    /* Property Declaration
     * ****************************************************************************************/
    required property UserPolicyTerms userPolicyTerms

    property bool isRead: false


    /* Object properties
     * ****************************************************************************************/
    title: "Privacy Policy & Terms Of Use "
    titleHeadingLevel: 5
    height: parent.height * 0.95
    width: parent.width * 0.95

    /* Children
     * ****************************************************************************************/

    Flickable {
        id: privacyFlick

        anchors.fill: parent
        anchors.bottomMargin: 10

        ScrollIndicator.vertical: ScrollIndicator {
            parent: privacyFlick
            height: parent.height
            x: parent.width

            onPositionChanged: {
                if (!isRead)
                    isRead = position > 0.98;
            }
        }

        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: privacyPolicyLabel.implicitHeight

        Label {
            id: privacyPolicyLabel
            anchors.fill: parent

            text: "### Privacy Policy V. " + userPolicyTerms.currentVersion + "\n\n" +
                  userPolicyTerms.privacyPolicy + "\n\n\n\n" +
                  "### Terms Of Use V. " + userPolicyTerms.currentVersion + "\n\n" +
                  userPolicyTerms.termsOfUse

            leftPadding: 4;
            rightPadding: 4
            background: null
            textFormat: Text.MarkdownText
            wrapMode: Text.WordWrap
            lineHeight: 1.3
            font.pointSize: Qt.application.font.pointSize * 0.7
        }

        //! Accept button
        // ButtonInverted {

        // }
    }
}
