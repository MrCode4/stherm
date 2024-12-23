import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.Dialogs

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

    ErrorPopup {
        id: emergencyModeErrorPopup

        errorMessage: {
            //! Show an error popup
            var remainigTime = ""
            if (uiSession.remainigTimeToUnblockSystemMode < 60000) {
                var seconds = (uiSession.remainigTimeToUnblockSystemMode / 1000).toFixed(0)
                remainigTime = `${seconds} second${(seconds > 1) ? "s" : ""}`;

            } else if (uiSession.remainigTimeToUnblockSystemMode < 3600000) {
                var minutes = Math.floor(uiSession.remainigTimeToUnblockSystemMode / 60000);
                var seconds = ((uiSession.remainigTimeToUnblockSystemMode  - minutes * 60000) / 1000).toFixed(0);
                remainigTime = `${minutes} minute${(minutes > 1) ? "s" : ""}`;
                if (seconds > 0) {
                    remainigTime += ` ${seconds} second${(seconds > 1) ? "s" : ""}`;
                }
            }
            if (uiSession.remainigTimeToUnblockSystemMode <= 0) {
                close();
                aboutToHide();
            }

            return `System mode change blocked due to emergency mode. Will resume in ${remainigTime}.`;

        }
    }

    SendingLogPopup{
        id:sendingLogPopup
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
            uiSession.openPageFromHome("qrc:/Stherm/View/SystemUpdatePage.qml");
        }
    }

    //! Show when the NRF update started.
    UpdatePopup {
        id: updatePopup
    }

    SuccessPopup {
        id: successPopup
    }

    property ConfirmPopup _systemSetupConfirmationPopup: null

    Component {
        id: _systemSetupConfirmationComponent

        ConfirmPopup {
            property var systemSetup

            title: "Update system setup"
            message: ""
            detailMessage: "The server and device system setups are different. Do you want to update the device to match the server?<br>Be sure to check the changes after the applying."
            buttons: MessageDialog.Cancel | MessageDialog.Apply
            keepOpen: true

            onButtonClicked: button => {
                                 if (button === MessageDialog.Apply) {
                                     deviceController.applySystemSetupServer(systemSetup);
                                     Qt.callLater(uiSession.openPageFromHome, "qrc:/Stherm/View/SystemSetupPage.qml");
                                 }
                             }

            onClosed: {
                _systemSetupConfirmationPopup.destroy();
                _systemSetupConfirmationPopup = null;
            }
        }
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
                font.pointSize: Qt.application.font.pointSize * 0.7
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
            PerfTestService.eligibilityChecked.connect(popupPerfTest.onPerfTestEligibilityChecked);
            PerfTestService.checkTestEligibilityManually("Menu");
        }
    }

    property SkipWIFIConnectionPopup _skipWIFIConnectionPopup: null

    Component {
        id: skipWIFIConnectionPopupComponent

        SkipWIFIConnectionPopup {
            onSkipWiFi: {
                deviceController.setInitialSetupNoWIFI(true);
            }
        }
    }

    property InitialFlowErrorPopup _initialFlowErrorPopup: null
    Component {
        id: initialFlowErrorPopupComponent

        InitialFlowErrorPopup {
            deviceController: root.uiSession.deviceController
            isBusy: deviceController.isSendingInitialSetupData

            onStopped: {
                deviceController.isSendingInitialSetupData = false;
            }
        }
    }

    property LimitedInitialSetupPopup _limitedInitialSetupPopup: null
    Component {
        id: limitedInitialSetupPopupComponent

        LimitedInitialSetupPopup {
            uiSession: root.uiSession
            remainigTime: deviceController.limitedModeRemainigTime
        }
    }

    Component {
        id: countDownPopUp

        CountDownPopup {
            property var callback

            anchors.centerIn: T.Overlay.overlay

            onStartAction: {
                if (callback instanceof Function) {
                    callback()
                }
            }

            onClosed: {
                destroy(this);
            }
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

                uiSession.popupLayout.displayPopUp(updateNotificationPopup);
            }
        }

        function onLogSentSuccessfully() {
            showSendingLogProgress();
        }

        function onLogAlert(message: string) {
            sendingLogPopup.close();
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

    Connections {
        target: deviceController

        function onShowEmergencyModeError() {
            uiSession.popupLayout.displayPopUp(emergencyModeErrorPopup);
        }

        function onShowInitialSetupPushError(err: string) {
            if (!_initialFlowErrorPopup) {
                _initialFlowErrorPopup = initialFlowErrorPopupComponent.createObject(root);
            }

            if (_initialFlowErrorPopup) {
                _initialFlowErrorPopup.errorMessage = err;
                uiSession.popupLayout.displayPopUp(_initialFlowErrorPopup);
            }
        }

        function onLimitedModeRemainigTimeChanged() {
            if (deviceController.limitedModeRemainigTime <= 0) {
                showLimitedInitialSetupPopup();
            }
        }
    }
    function showCountDownPopUp(title: string, actionText: string, hasCancel: bool, callback: Function) {
        var popup = countDownPopUp.createObject(root, {
                                                    "title": title,
                                                    "actionText": actionText,
                                                    "cancelEnable": hasCancel,
                                                    "callback": callback
                                                })
       uiSession.popupLayout.displayPopUp(popup);
        return popup
    }

    function warrantyReplacementFinished() {
        deviceController.firstRunFlowEnded();
        successPopup.hid.disconnect(warrantyReplacementFinished);
    }

    //! Get skip wifi connection popup object
    function showSkipWIFIConnectionPopup() {
        if (!_skipWIFIConnectionPopup) {
            _skipWIFIConnectionPopup = skipWIFIConnectionPopupComponent.createObject(root);
        }

        if (_skipWIFIConnectionPopup)
            uiSession.popupLayout.displayPopUp(_skipWIFIConnectionPopup);
    }

    function showLimitedInitialSetupPopup() {
        if (!_limitedInitialSetupPopup) {
            _limitedInitialSetupPopup = limitedInitialSetupPopupComponent.createObject(root);
        }

        if (_limitedInitialSetupPopup) {
            uiSession.popupLayout.displayPopUp(_limitedInitialSetupPopup);
        }
    }

    function initSendingLogProgress() {
        sendingLogPopup.init();
        showSendingLogProgress();
    }

    function showSendingLogProgress() {
        if (!sendingLogPopup.visible)
            uiSession.popupLayout.displayPopUp(sendingLogPopup);
    }

    function showSystemSetupUpdateConfirmation(settings: var) {
        if (!_systemSetupConfirmationPopup) {
            _systemSetupConfirmationPopup = _systemSetupConfirmationComponent.createObject(root);
        }

        if (_systemSetupConfirmationPopup) {
            _systemSetupConfirmationPopup.systemSetup = settings;
            uiSession.popupLayout.displayPopUp(_systemSetupConfirmationPopup);
        }
    }
}
