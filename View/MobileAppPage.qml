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

    /* Children
     * ****************************************************************************************/
    GridLayout {
        anchors.centerIn: parent
        width: parent.width
        columnSpacing: 5
        columns: 2

        Label {
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            text: "1."
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
            MouseArea {
                anchors.fill: parent
                onClicked: fetchUserData()
                Component.onCompleted: fetchUserData()

                function fetchUserData() {
                    uiSession.deviceController.deviceControllerCPP.sync.fetchUserData();
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            text: "2."
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
            visible: !uiSession.appModel.userData.email
        }

        Label {
            visible: uiSession.appModel.userData.email
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width - 10
            text: "To recover your password, click on\n“Forgot password”, enter the email\nbelow, create a password and log in."
            font.pointSize: Qt.application.font.pointSize * 0.9
        }

        Label {
            visible: uiSession.appModel.userData.email
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            Layout.topMargin: 20
            Layout.columnSpan: 2
            elide: Text.ElideMiddle
            text: "Email: " + uiSession.appModel.userData.email
        }
    }    
}
