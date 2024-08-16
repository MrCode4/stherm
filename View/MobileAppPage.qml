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
            text: "To recover your password, click on\n“Forgot password”, enter the email\nbelow, create a password and log in."
            font.pointSize: Qt.application.font.pointSize * 0.9
        }

        Label {
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: parent.width
            Layout.topMargin: 20
            Layout.columnSpan: 2
            elide: Text.ElideMiddle
            text: "Email: " + uiSession.deviceController.deviceControllerCPP.sync.userData.email
        }
    }    
}
