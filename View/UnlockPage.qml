import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UnlockPage: unLock the app
 * ***********************************************************************************************/

BasePageView {
    id: root

    //! Use in unlock page
    property string encodedMasterPin: ""

    property bool showUnlockEmergency: deviceController.initialSetupNoWIFI

    /* Object properties
     * ****************************************************************************************/


    title: ""
    backButtonVisible: false
    header: null

    Component.onCompleted: {
        generateMasterPin();
    }

    //! Wifi status
    WifiButton {
        id: _wifiBtn

        anchors.right: parent.right
        anchors.top: parent.top
        z: 1

        visible: !Boolean(NetworkInterface.connectedWifi) ||
                 !Boolean(NetworkInterface.hasInternet)

        onClicked: {
            //! Open WifiPage
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/WifiPage.qml", {
                                             "uiSession": uiSession
                                         });
            }
        }
    }

    //! technician access page
    ToolButton {
        touchMargin: 30
        visible: showUnlockEmergency

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 1

        contentItem: RoniaTextIcon {
            anchors.fill: parent
            font.pointSize: Style.fontIconSize.largePt
            text: FAIcons.headSet
        }

        onClicked: {
            //! Open technician access page
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/UnlockEmergencyPage.qml", {
                                             "uiSession": uiSession,
                                             "openFromUnlockPage": true,
                                             "encodedMasterPin": root.encodedMasterPin
                                         });
            }
        }
    }

    //! Contents
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 8 * scaleFactor
        spacing: 8

        Image {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            width: 37
            height: 49
            source: "qrc:/Stherm/Images/lock-image.svg"
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: root.font.pointSize * 0.8
            text: "Type the Pin to Unlock"
            elide: Text.ElideMiddle
        }

        PINKeyboard {
            id: pinItem

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            isLock: false

            onForgetPIN: {
                showUnlockEmergency = true;
                generateMasterPin();
            }

            onSendPIN: pin => {
                           let unLocked = deviceController.updateAppLockState(false, pin);
                           updatePinStatus(unLocked);
                           clearPIN();
                           if (unLocked) {
                               showUnlockEmergency = false;
                           }
                       }
        }
    }

    function generateMasterPin() {
        if (root.encodedMasterPin.length === 6 &&
                appModel.lock._masterPIN.length === 4) {
            return;
        }

        let randomPin = AppUtilities.generateRandomPassword();
        root.encodedMasterPin = randomPin;
        appModel.lock._masterPIN = AppUtilities.decodeLockPassword(randomPin);
    }
}
