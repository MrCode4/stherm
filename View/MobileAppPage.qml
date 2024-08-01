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
    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width

        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            text: "1. Download and install the Nuve Home app."
            wrapMode: Text.WordWrap
        }

        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: parent.width
            fillMode: Image.PreserveAspectFit
            source: "qrc:/Stherm/Images/mobile-app-link.png"
        }

        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            text: "2. To recover your password, click on “Forgot password”, enter the email below, create a new password and log in."
             wrapMode: Text.WordWrap
        }

        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            Layout.topMargin: 20
            text: "Email: [email]@mail.com"
        }
    }
}
