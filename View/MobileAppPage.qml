import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * MobileAppPage: mobile app page
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Mobile App"

    RowLayout {
        parent: root.header.contentItem
        Item {
            Layout.alignment: Qt.AlignCenter
            implicitWidth: Material.touchTarget
            implicitHeight: Material.touchTarget

            ToolButton {
                id: btnRefresh
                anchors.centerIn: parent
                visible: uiSession.deviceController.deviceControllerCPP.sync.fetchingUserData == false
                contentItem: RoniaTextIcon {
                    text: "\uf2f9" //! rotate-right
                }

                onClicked: root.fetchUserData()
            }

            BusyIndicator {
                anchors {
                    fill: parent
                    margins: 4
                }
                visible: running
                running: !btnRefresh.visible
            }
        }
    }

    function fetchUserData() {
        uiSession.deviceController.deviceControllerCPP.sync.fetchUserData();
    }

    Component.onCompleted: fetchUserData()


    /* Children
     * ****************************************************************************************/
    GridLayout {
        anchors.centerIn: parent
        width: parent.width
        columns: 2

        Label {
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            text: "1. "
            wrapMode: Text.NoWrap
            font.pointSize: Qt.application.font.pointSize * 0.9
        }
        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width - 10
            text: "Download and install the Nuve Home app."
            wrapMode: Text.WordWrap
            font.pointSize: Qt.application.font.pointSize * 0.9
        }

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width
            Layout.columnSpan: 2
            fillMode: Image.PreserveAspectFit
            source: "qrc:/Stherm/Images/mobile-app-link.png"
        }

        Label {
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            text: "2. "
            wrapMode: Text.NoWrap
            font.pointSize: Qt.application.font.pointSize * 0.9
        }
        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width - 10
            text: "To recover your password, please go to mobile application, click on “Forgot password”, " +
                  "enter the email you have provided during installation, create a new password and log in."
            wrapMode: Text.WordWrap
            font.pointSize: Qt.application.font.pointSize * 0.9
        }
        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            visible: false // as we have no info for real email
            text: "To recover your password, click on “Forgot password”, enter the email below, create a new password and log in."
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            Layout.topMargin: 20
            Layout.columnSpan: 2
            text: "Email: " + uiSession.deviceController.deviceControllerCPP.sync.userData.email
        }
    }    
}
