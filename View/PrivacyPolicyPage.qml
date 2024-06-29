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


    /* Object properties
     * ****************************************************************************************/

    title: "Privacy Policy and Terms Of Use"

    backButtonVisible: !testMode

    Component.onCompleted: {
        //! Load privacy policy text
        privacyPolicyLabel.text = AppSpec.readFromFile(":/Stherm/Resources/privacyPolicy.md");

        //! Load terms of usae text
        termsUsageLabel.text = AppSpec.readFromFile(":/Stherm/Resources/termOfUse.md");
    }

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
            //! Load next page
            if (root.StackView.view) {
                if (testMode) {
                    root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "backButtonVisible" : false
                                             });

                } else {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                             });
                }

            }
        }
    }

    //! Privacy Policy
    ExpandableItem {
        id: privacyPolicyExpand

        anchors.top: parent.top

        width: parent.width
        maxHeight: root.height * 0.55

        isExpanded: true
        title: "Privacy Policy"

        onIsExpandedChanged: {
            termsOfUseExpand.isExpanded = !isExpanded;
        }

        Item {
            anchors.fill: parent

            Flickable {
                id: privacyFlick

                property bool isRead: false

                ScrollIndicator.vertical: ScrollIndicator {
                    parent: privacyFlick.parent
                    height: parent.height
                    x: parent.width

                    onPositionChanged: {
                        if (!privacyFlick.isRead)
                            privacyFlick.isRead = position > 0.95;
                    }
                }

                anchors.fill: parent
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: width
                contentHeight: privacyPolicyLabel.implicitHeight

                Label {
                    id: privacyPolicyLabel
                    anchors.fill: parent
                    leftPadding: 4;
                    rightPadding: 4
                    background: null
                    textFormat: Text.MarkdownText
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                    font.pointSize: Qt.application.font.pointSize * 0.7
                }
            }
        }
    }

    //! Term Of Use
    ExpandableItem {
        id: termsOfUseExpand

        anchors.top: privacyPolicyExpand.bottom
        anchors.topMargin: 10
        isExpanded: false
        title: "Term Of Use"
        width: parent.width
        maxHeight: root.height * 0.55

        onIsExpandedChanged: {
            privacyPolicyExpand.isExpanded = !isExpanded;
        }

        Item {
            anchors.fill: parent


            Flickable {
                id: termsflick

                property bool isRead: false

                ScrollIndicator.vertical: ScrollIndicator {
                    parent: termsflick.parent
                    height: parent.height
                    x: parent.width

                    onPositionChanged: {
                        if (!termsflick.isRead)
                            termsflick.isRead = position > 0.95;
                    }
                }


                anchors.fill: parent
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                contentWidth: width
                contentHeight: termsUsageLabel.implicitHeight

                Label {
                    id: termsUsageLabel
                    anchors.fill: parent
                    leftPadding: 4;
                    rightPadding: 4
                    background: null
                    textFormat: Text.MarkdownText
                    wrapMode: Text.WordWrap
                    lineHeight: 1.3
                    font.pointSize: Qt.application.font.pointSize * 0.7
                }
            }
        }
    }

    RowLayout {
        spacing: 4

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10

        enabled: privacyFlick.isRead && termsflick.isRead
        CheckBox {
            id: privacyPolicyChbox
            Layout.leftMargin: -leftPadding
            focusPolicy: Qt.TabFocus
        }

        Label {
            Layout.alignment: Qt.AlignVCenter

            font.pointSize: Application.font.pointSize * 0.85
            textFormat: Text.StyledText
            linkColor: Material.foreground
            verticalAlignment: "AlignVCenter"
            text: '<p>I agree to <b><a>Privacy Policy</a></b> and <b><a>Terms of use</a></b>.</p>'

            TapHandler {
                onTapped: {
                    privacyPolicyChbox.checked = !privacyPolicyChbox.checked;
                }
            }
        }
    }
}
