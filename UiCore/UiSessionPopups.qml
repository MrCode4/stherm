import QtQuick
import QtQuick.Layouts

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
 * \todo The Popups should be passed the uiSession and handle their own actions instead of leaving
 *       that to the parent class.
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

    //! Switch heating mode in the dual fuel heating
    property alias dfhSwitchHeatingPopup:      switchHeatingPopup

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

    //! Used in the dual fuel heating
    SwitchHeatingPopup {
        id: switchHeatingPopup

        deviceController: _root.deviceController
        onOpenSystemModePage: uiSession.openSystemModePage();
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
            parent.popupLayout.displayPopUp(installConfirmation);

            // Active screen saver
            ScreenSaverManager.setActive();
        }

        function onError(err) {
            console.log("Update error: ", err);
            if (downloadingPopup.visible)
                downloadingPopup.close();

            parent.popupLayout.displayPopUp(updateInterruptionPopup);

            // Active screen saver
            ScreenSaverManager.setActive();
        }

        function onDownloadStarted() {
            if (!downloadingPopup.visible)
                parent.popupLayout.displayPopUp(downloadingPopup);

            // Inactive screen saver
            ScreenSaverManager.setInactive();
        }

        function onNotifyNewUpdateAvailable() {
            if (system.updateAvailable) {
                //! mark update as mandatory after initial setup done!
                deviceController.mandatoryUpdate = deviceController.deviceControllerCPP.system.isInitialSetup();

                updateNotificationPopup.open();
            }
        }

        function onWarrantyReplacementFinished(success: bool) {
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
                parent.popupLayout.displayPopUp(updatePopup);

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
