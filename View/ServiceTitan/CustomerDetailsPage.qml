import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ThermostatNamePage
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
    ToolButton {
        parent: root.header.contentItem
        checkable: false
        checked: false
        visible: initialSetup
        implicitWidth: 64
        implicitHeight: implicitWidth
        icon.width: 50
        icon.height: 50

        contentItem: RoniaTextIcon {
            anchors.fill: parent
            font.pointSize: Style.fontIconSize.largePt
            Layout.alignment: Qt.AlignLeft
            text: FAIcons.circleInfo
        }

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
        spacing: 4

        Label {
            text: "Email"
            font.pointSize: root.font.pointSize * 1.1
        }

        TextField {
            id: emailTf

            Layout.fillWidth: true
            placeholderText: "Email"
            text: appModel?.serviceTitan?.email ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }
        }

        Label {
            Layout.fillWidth: true

            width: parent.width
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.7
            text: "Email will be used as a Mobile App login credential."
        }

        Item {
            id: spacer
            Layout.fillWidth: true
            height: 20
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize * 1.1
        }

        TextField {
            id: zipCodeTf

            Layout.fillWidth: true

            placeholderText: "ZIP code"
            text: appModel?.serviceTitan?.zipCode ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
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
        enabled: emailTf.text.length > 0 && zipCodeTf.text.length > 0
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
