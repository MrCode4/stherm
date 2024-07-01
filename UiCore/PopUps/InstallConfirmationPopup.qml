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

    //! isBackdoor used to install the backdoor updates.
    property bool isBackdoor: false

    //! isResetToVersion
    property bool isResetToVersion: false

    //! isFWServerVersion: update framware from server
    property bool isFWServerVersion: false

    /* Object properties
     * ****************************************************************************************/

    title: ""
    closePolicy: Popup.NoAutoClose
    titleBar: false


    onOpened: {
        installUpdate();
    }

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay

        anchors.fill: parent
        anchors.margins: 8

        spacing: 32

        RoniaTextIcon {
            id: icon

            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: FAIcons.restart
        }

        Label {
            id: restartingLabel

            Layout.fillWidth: true
            Layout.preferredWidth: 250
            font.pointSize: Application.font.pointSize
            wrapMode: Text.WordWrap
            text: "Restarting..."
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

    }

    //! Install update and restart the app.
    function installUpdate() {
        // Inactive screen saver
        ScreenSaverManager.setInactive();

        // Restart the app.
        deviceController.deviceControllerCPP.system.updateAndRestart(isBackdoor, isResetToVersion, isFWServerVersion);
    }
}
