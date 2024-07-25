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
            text: "1. Download and install the Nuve Home application."
            wrapMode: Text.WordWrap
        }

        Image {
            Layout.alignment: Qt.AlignHCenter
            source: "qrc:/Stherm/Images/mobile-app-link.png"
        }

        Label {
            Layout.alignment: Qt.AlignLeft
            text: "2. Click on “Forgot password” then enter the email address below to create a new password and log in."
             wrapMode: Text.WordWrap
        }

        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.topMargin: 10
            text: "Email: [email]@mail.com"
        }
    }
}
