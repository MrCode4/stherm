import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UiSessionPopups is a helper class to contain all the popups that can be called throughout the
 * RIG GUI (but relate to the session). This way we only need to instantiate one instance of the
 * same popup, which should improve performance, and, most of all, warrant consistency.
 *
 * \todo The current implementation is limited/questionable as uiSession cannot be passed to the
 *       popups. In reality, the popups belog the the Stherm, not to the session, and should be
 *       moved there. The question then remains how the communication should work.
 *
 * \todo A better API could allow any class to call uiSession.popUps.xyz.show(), or to simply call
 *       uiSession.actions.doSomething(args);
 *
 * ************************************************************************************************/
Item {
    id: root

    /* Property declartion
     * ****************************************************************************************/
    property var uiSession

    property DeviceController deviceController: uiSession.deviceController

    property System system: deviceController.deviceControllerCPP.system

    //!
    property alias exitConfirmPopup:        exitConfPop

    //!
    property alias scheduleOverlapPopup:    schOverlapPop

    //!
    property alias errorPopup:              errorPop
    property alias perfTestCheckPopup: popupPerfTest

    readonly property bool isAnyPopupOpened : updateNotificationPopup.opened
                                                || successPopup.opened


    /* Signal Handlers
     * ****************************************************************************************/

    //! Open a page from home.
    signal openPageFromHome(item: string);

    /* Children
     * ****************************************************************************************/
    ExitConfirmPopup {
        id: exitConfPop
    }

    ScheduleOverlapPopup {
        id: schOverlapPop
    }

    ErrorPopup {
        id: errorPop
    }

    DownloadingPopup {
        id: downloadingPopup

        deviceController: root.deviceController
    }

    UpdateInterruptionPopup {
        id: updateInterruptionPopup

        deviceController: root.deviceController
    }

    InstallConfirmationPopup {
        id: installConfirmation

        deviceController: root.deviceController
    }

    UpdateNotificationPopup {
        id: updateNotificationPopup

        mandatoryUpdate: deviceController.mandatoryUpdate

        onOpenUpdatePage: {
            root.openPageFromHome("qrc:/Stherm/View/SystemUpdatePage.qml");
        }
    }

    //! Show when the NRF update started.
    UpdatePopup {
        id: updatePopup
    }

    SuccessPopup {
        id: successPopup
    }

    Popup {
        id: popupPerfTest
        parent: T.Overlay.overlay
        width: parent.width * 0.6
        anchors.centerIn: parent
        modal: true

        property bool isBusyMode: true
        property string message: "Checking Eligibility"

        contentItem: ColumnLayout {
            anchors.fill: parent
            spacing: 15

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 10
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                font.pointSize: root.font.pointSize * 0.7
                horizontalAlignment: Qt.AlignHCenter
                wrapMode: Text.WordWrap
                text: popupPerfTest.message
            }

            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                height: 45
                width: 45
                running: visible
                visible: popupPerfTest.isBusyMode
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                visible: !popupPerfTest.isBusyMode
                onClicked: popupPerfTest.close()
                text: "OK"
            }
        }

        function onPerfTestEligibilityChecked(errMsg) {
            PerfTestService.eligibilityChecked.disconnect(popupPerfTest.onPerfTestEligibilityChecked);
            if (!errMsg) {
                popupPerfTest.close();
                uiSession.showHome();
            }
            else {
                popupPerfTest.message = errMsg;
                popupPerfTest.isBusyMode = false;
            }
        }

        function checkPerfTestEligibility() {
            popupPerfTest.message = "Checking Eligibility";
            popupPerfTest.isBusyMode = true;
            popupPerfTest.open();
            PerfTestService.eligibilityChecked.connect(root.onPerfTestEligibilityChecked);
            PerfTestService.checkTestEligibility();
        }
    }

    //! Connections to show installConfirmation popup
    Connections {
        target: system

        function onPartialUpdateReady(isBackdoor : bool, isResetToVersion: bool, isFWServerVersion: bool) {
            if (downloadingPopup.visible)
                downloadingPopup.close();

            installConfirmation.isBackdoor = isBackdoor;
            installConfirmation.isResetToVersion = isResetToVersion;
            installConfirmation.isFWServerVersion = isFWServerVersion;
            uiSession.popupLayout.displayPopUp(installConfirmation);

            // Active screen saver
            ScreenSaverManager.setActive();
        }

        function onError(err) {
            console.log("Update error: ", err);
            if (downloadingPopup.visible)
                downloadingPopup.close();

            uiSession.popupLayout.displayPopUp(updateInterruptionPopup);

            // Active screen saver
            ScreenSaverManager.setActive();
        }

        function onDownloadStarted() {
            if (!downloadingPopup.visible)
                uiSession.popupLayout.displayPopUp(downloadingPopup);

            // Inactive screen saver
            ScreenSaverManager.setInactive();
        }

        function onNotifyNewUpdateAvailable() {
            if (system.updateAvailable) {
                //! mark update as mandatory after initial setup done!
                deviceController.mandatoryUpdate = deviceController.deviceControllerCPP.system.isInitialSetup() && deviceController.startMode === 1;

                updateNotificationPopup.open();
            }
        }
}
    Connections {
        target: deviceController.sync

        function onWarrantyReplacementFinished(success: bool, error: string, needToRetry: bool) {
            console.log("WarrantyReplacementFinished", success);

            if (success) {
                successPopup.message = "The new thermostat has successfully integrated into the system."
                successPopup.hid.connect(warrantyReplacementFinished);
                successPopup.open();
            }
            // how about in error?
        }
    }

    Connections {
        target: deviceController.deviceControllerCPP

        function onNrfUpdateStarted() {
            if (!updatePopup.visible)
                uiSession.popupLayout.displayPopUp(updatePopup);

            // Inactive screen saver
            ScreenSaverManager.setInactive();
        }

        function onNrfVersionChanged() {
            updatePopup.close();

            // Active screen saver
            ScreenSaverManager.setActive();
        }
    }

    function warrantyReplacementFinished() {
        deviceController.firstRunFlowEnded();
        successPopup.hid.disconnect(warrantyReplacementFinished);
    }
}
