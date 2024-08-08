import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UnlockPage: unLock the app
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
     property Lock lock: appModel._lock

    //! Use in unlock page
    property string encodedMasterPin: ""

    /* Object properties
     * ****************************************************************************************/
    title: ""
    backButtonVisible: false
    header: null

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
        id: contactContractorBtn

        touchMargin: 30
        visible: false

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
                root.StackView.view.push("qrc:/Stherm/View/UserGuidePage.qml", {
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
                contactContractorBtn.visible = true;
                if (root.encodedMasterPin.length === 8 &&
                        appModel._lock._masterPIN.length === 4)
                    return;

                var randomPin = AppSpec.generateRandomPassword();
                root.encodedMasterPin = randomPin;
                appModel._lock._masterPIN = AppSpec.decodeLockPassword(randomPin);
            }

            onSendPIN: pin => {
                           var unLocked = deviceController.lock(false, pin);
                           updatePinStatus(unLocked);

                           clearPIN();

                           if (unLocked)
                               contactContractorBtn.visible = false;
                       }
        }
    }
}
