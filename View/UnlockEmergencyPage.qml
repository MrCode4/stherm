import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UnlockEmergencyPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    objectName: "UnlockEmergency"

    /* Object Declaration
     * ****************************************************************************************/

    property System system: deviceController.deviceControllerCPP.system

    //! Use in unlock page
    property bool openFromUnlockPage: false

    //! Use in unlock page
    property string encodedMasterPin: ""

    property string supportNumber: "(657) 626-4887"

    /* Object properties
     * ****************************************************************************************/
    title: qsTr("Unlock the thermostat")

    /* Children
     * ****************************************************************************************/

    Column {
        spacing: 15
        visible: system.serialNumber.length > 0

        anchors{
            left: parent.left
            leftMargin: 30
            right: parent.right
            rightMargin: 30
            top:parent.top
            topMargin: 60
        }

        Label {
            wrapMode: Text.WordWrap
            horizontalAlignment: Qt.AlignHCenter
            text: qsTr("You can unlock the thermostat from the Unlock page in your Mobile app’s menu.
%1
If you don't have access to the Mobile app, please contact our tech support at
%2 and provide the code below to retrieve the master PIN for unlocking the thermostat.")
            .arg("") // empty line
            .arg(root.supportNumber)
            anchors {
                left: parent.left
                right: parent.right
            }

            font{
                pointSize: Application.font.pointSize * 0.9
            }
        }

        Label {
            topPadding: 10
            width: parent.width
            visible: openFromUnlockPage
            horizontalAlignment: Qt.AlignHCenter
            text: "YOUR CODE: " + root.encodedMasterPin

            font{
                pointSize: Application.font.pointSize * 0.9
                bold: true
            }
        }
    }
}
