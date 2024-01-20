import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InstallConfirmationPopup start the update procedure and request user authorization to restart the application..
 * ***********************************************************************************************/

I_PopUp {
    /* Property Declaration
     * ****************************************************************************************/

    property DeviceController deviceController

    property bool             restaring: deviceController.deviceControllerCPP.system.isForceUpdate

    /* Object properties
     * ****************************************************************************************/

    title: ""
    closePolicy: restaring ? Popup.NoAutoClose : (Popup.CloseOnReleaseOutside | Popup.CloseOnEscape)
    titleBar: !restaring

    onClosed: restaring = false;

    onOpened: {
        if (restaring) {
            installUpdate();
        }
    }


    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        width: parent?.width ?? 0
        anchors.centerIn: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 32

        RoniaTextIcon {
            id: icon

            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: restaring ? FAIcons.restart : FAIcons.circleCheck
        }

        Label {

            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.75
            wrapMode: Text.WordWrap
            text: "Updates has been successefully downloaded.\nThe application update requires a restart. Would you like to proceed?"
            horizontalAlignment: Text.AlignLeft
            visible: !restaring
        }

        Label {
            id: restartingLabel

            visible: restaring
            Layout.fillWidth: true
            Layout.preferredWidth: 120
            font.pointSize: Application.font.pointSize
            wrapMode: Text.WordWrap
            text: "Restarting..."
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }


        RowLayout {
            id: rowButton

            Layout.fillWidth: true

            Item {
                Layout.fillWidth: true
            }

            ButtonInverted {
                id: okButton

                Layout.fillHeight: true
                leftPadding: 8
                rightPadding: 8
                text: "Ok"
                visible: !restaring

                onClicked: {
                    restaring = true;

                    installUpdate();
                }
            }
        }

    }

    //! Install update and restart the app.
    function installUpdate() {
        // Inactive screen saver
        ScreenSaverManager.setInactive();

        // Restart the app.
        deviceController.deviceControllerCPP.system.updateAndRestart();
    }
}
