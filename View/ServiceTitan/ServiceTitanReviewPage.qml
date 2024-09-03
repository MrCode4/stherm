import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ServiceTitanReviewPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false


    /* Object properties
     * ****************************************************************************************/
    title: "Review"

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

    Label {
        id: confirmInfoLabel

        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.9
        text: "Please confirm with the customer that the information below is correct."
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }

    ColumnLayout {
        anchors.top: confirmInfoLabel.bottom
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        spacing: 4

        Label {
            text: "Email"
            font.pointSize: root.font.pointSize * 1.1
        }

        Label {

            id: emailTf

            text: appModel?.serviceTitan?.email ?? ""
            font.pointSize: Application.font.pointSize * 0.9
        }

        Item {
            Layout.fillWidth: true
            height: 20
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize * 1.1
        }

        Label {
            id: zipCodeTf

            Layout.fillWidth: true

            text: appModel?.serviceTitan?.zipCode ?? ""
            font.pointSize: Application.font.pointSize * 0.9
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
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            // Go to next page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/InstallationTypePage.qml", {
                                              "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                          });
            }
        }
    }

}
