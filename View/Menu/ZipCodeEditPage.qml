import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ZipCodeEditPage provides ui to edit the country and related zip code.
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property ServiceTitan serviceTitan: appModel.serviceTitan

    property bool          isBusyZip:    false
    property bool          isBusyUpdatingAddress:    false

    readonly property bool isBusy: root.isBusyZip || root.isBusyUpdatingAddress

    property bool          hasChange: {
        let changes = zipCodeTf.text.toUpperCase() !== serviceTitan.zipCode.toUpperCase();
        changes |= serviceTitan?.country !== countryCombobox.currentText;

        return changes;
    }

    //! To prevent closing the page when an active process is running.
    onIsBusyChanged: {
        if (root.isBusy) {
            ScreenSaverManager.setInactive();

        } else {
            ScreenSaverManager.setActive();
        }
    }

    /* Object properties
     * ****************************************************************************************/
    title: "Location Hub"
    leftPadding: 10 * scaleFactor
    rightPadding: 10 * scaleFactor
    backButtonEnabled: !root.isBusy

    backButtonCallback: function() {
        //! Check if the user data changed
        if (hasChange && zipCodeTf.acceptableInput) {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(goBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);

        } else {
            goBack();
        }
    }


    /* Children
     * ****************************************************************************************/

    ToolButton {
        id: confirmtBtn
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        enabled: hasChange && zipCodeTf.acceptableInput

        onClicked: {
            getZipCodeInformationTryTimer.triggered();
        }
    }

    GridLayout {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter

        width: root.availableWidth

        columns: 2
        columnSpacing: 15
        rowSpacing: 0

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            font.pointSize: root.font.pointSize * 0.95
            text: "The zip code provided for your address is used to display accurate outdoor temperature information.\nPlease enter the correct information.\n"
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Label {
            text: "Country"
            font.pointSize: root.font.pointSize * 0.9
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize * 0.9
        }

        ComboBox {
            id: countryCombobox

            Layout.preferredWidth: root.availableWidth / 2 - 10
            Layout.alignment: Qt.AlignBottom

            font.pointSize: root.font.pointSize * 0.8
            model: AppSpec.supportedCountries
            currentIndex: appModel?.serviceTitan?.country.length > 0 ?
                              AppSpec.supportedCountries.indexOf(appModel?.serviceTitan?.country) : 0
        }

        TextField {
            id: zipCodeTf

            Layout.preferredWidth: root.availableWidth  / 2 - 10

            placeholderText: "Input the ZIP code"
            text: appModel?.serviceTitan?.zipCode.toUpperCase() ?? ""
            font.pointSize: root.font.pointSize * 0.8

            // Australia: 4 digits
            // Canada: 6 letters + digits
            // US: 5 digits: 10498
            validator: RegularExpressionValidator {
                regularExpression: /^(?:\d{4,5}|[A-Z\d]{6})$/i
            }

            inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhPreferUppercase
        }
    }

    Connections {
        target: deviceController
        enabled: root.visible && root.isBusyZip

        function onZipCodeInfoReady(error, isNeedRetry) {
            root.isBusyZip = isNeedRetry && error.length > 0;

            console.log("MAK sdf", error)
            if (error.length > 0) {
                errorPopup.errorMessage = "ZIP code information is not ready, " + error;

                if (isNeedRetry) {
                    getZipCodeInformationTryTimer.start();
                }

                if ((getZipCodeInformationTryTimer.retryCounter % 2 === 0) || !isNeedRetry) {
                    errorPopup.open();
                }

            } else {
                getZipCodeInformationTryTimer.stop();

                serviceTitan.zipCode= zipCodeTf.text.toUpperCase();
                serviceTitan.country = countryCombobox.currentText;

                //! Update address in the server
                updateAddressTryTimer.triggered();
            }
        }
    }

    Connections {
        target: deviceController.sync
        enabled: root.visible && root.isBusyUpdatingAddress

        function onClientAddressUpdatingFinished(success: bool, error: string, isNeedRetry: bool) {
            root.isBusyUpdatingAddress = !success && isNeedRetry && error.length > 0;

            if (error.length > 0) {
                errorPopup.errorMessage = "Update address information failed, " + error;

                if (isNeedRetry) {
                    updateAddressTryTimer.start();
                }

                if ((updateAddressTryTimer.retryCounter % 2 === 0) || !isNeedRetry) {
                    errorPopup.open();
                }

            } else {
               goBack();
            }
        }
    }

    Timer {
        id: getZipCodeInformationTryTimer

        interval: 5000
        repeat: false
        running: false

        property int retryCounter: 0

        onTriggered: {
            retryCounter++;
            root.isBusyZip = true;
            deviceController.getZipCodeJobInformationManual(zipCodeTf.text.toUpperCase());
        }
    }

    Timer {
        id: updateAddressTryTimer

        interval: 5000
        repeat: false
        running: false

        property int retryCounter: 0

        onTriggered: {
            root.isBusyUpdatingAddress = true;
            retryCounter++;
            deviceController.updateAddressInformation();
        }
    }

    BusyIndicator {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter

        height: 45
        width: 45
        visible: root.isBusy
        running: visible
    }

    CriticalErrorDiagnosticsPopup {
        id: errorPopup

        enableSendLog: false
        isBusy: root.isBusy
        deviceController: uiSession.deviceController

        onStopped: {
            root.isBusyZip = false;
            getZipCodeInformationTryTimer.stop();

            root.isBusyUpdatingAddress = false;
            updateAddressTryTimer.stop();
        }
    }

    /* Functions
     * ****************************************************************************************/

    //! This method is used to go back
    function goBack()
    {
        // To ensure the screen saver is active.
        ScreenSaverManager.setActive();

        uiSession.popUps.exitConfirmPopup.accepted.disconnect(confirmtBtn.clicked);
        uiSession.popUps.exitConfirmPopup.rejected.disconnect(goBack);

        if (root.StackView.view) {
            //! Then Page is inside an StackView
            if (root.StackView.view.currentItem == root) {
                root.StackView.view.pop();
            }
        }
    }
}
