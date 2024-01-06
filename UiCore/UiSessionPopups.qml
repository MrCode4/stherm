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
    property DeviceController deviceController: parent.deviceController

    //!
    property alias exitConfirmPopup:        exitConfPop

    //!
    property alias scheduleOverlapPopup:    schOverlapPop

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

        onOpenUpdatePage: {
            root.openPageFromHome("qrc:/Stherm/View/SystemUpdatePage.qml");
        }
    }

    //! Connections to show installConfirmation popup
    Connections {
        target: deviceController.deviceControllerCPP.system

        function onPartialUpdateReady() {
            if (downloadingPopup.visible)
                downloadingPopup.close();

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
            if (deviceController.deviceControllerCPP.system.updateAvailable)
                updateNotificationPopup.open();
        }
    }
}
