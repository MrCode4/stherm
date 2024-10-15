import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ServiceTitanReviewPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property bool fetchMnaual: appModel?.serviceTitan?.isSTManualMode ?? true

    //! Busy due to multiple api calls
    property bool isBusyZip: false
    property bool isBusyCustomer: false
    readonly property bool isBusy: isBusyZip || isBusyCustomer

    /* Object properties
     * ****************************************************************************************/
    title: "Review"

    onVisibleChanged: {
        if (!visible) {
            errorPopup.close();
            retryTimer.stop();
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Info button in initial setup mode.
    InfoToolButton {
        parent: root.header.contentItem
        visible: initialSetup

        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                             "uiSession": Qt.binding(() => uiSession)
                                         });
            }

        }

        //! Wifi status
        WifiButton {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.bottom
            anchors.topMargin: -10
            visible: !NetworkInterface.hasInternet

            z: 1

            onClicked: {
                //! Open WifiPage
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/WifiPage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup,
                                                 "nextButtonEnabled": false
                                             });
                }
            }
        }
    }

    Label {
        id: confirmInfoLabel

        anchors.top: parent.top
        anchors.topMargin: 15
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.7
        text: "Please confirm with the customer that the information below is correct."
        font.pointSize: Application.font.pointSize * 0.9
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }

    ColumnLayout {
        anchors.top: confirmInfoLabel.bottom
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        spacing: 4

        Label {
            text: "Email"
            font.pointSize: root.font.pointSize
        }

        Label {

            id: emailTf

            text: appModel?.serviceTitan?.email ?? ""
            font.pointSize: Application.font.pointSize * 0.9
        }

        Item {
            Layout.fillWidth: true
            height: 20
        }

        Label {
            text: "ZIP code"
            font.pointSize: root.font.pointSize
        }

        Label {
            id: zipCodeTf

            Layout.fillWidth: true

            text: appModel?.serviceTitan?.zipCode ?? ""
            font.pointSize: Application.font.pointSize * 0.9
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }


    Label {
        anchors.bottom: nextBtn.top
        anchors.bottomMargin: 10
        anchors.left: parent.left
        width: parent.width

        font.pointSize: Application.font.pointSize * 0.7
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment:Text.AlignBottom
        text: "Contact Nuve Support: (657) 626-4887 for issues."
    }

    //! Next button
    ButtonInverted {
        id: nextBtn

        enabled: !isBusy

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        text: "Next"
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            retryTimer.retryZIPCounter = 0;
            retryTimer.retryEmailCounter = 0;

            if (NetworkInterface.hasInternet) {
                isBusyZip = true;
                // get needed values from api
                deviceController.getZipCodeJobInformationManual();

            } else {
                errorPopup.errorMessage = "No internet connection. Please check your internet connection.";
                errorPopup.open();
            }
        }
    }

    BusyIndicator {
        anchors.right: nextBtn.left
        anchors.verticalCenter: nextBtn.verticalCenter

        height: 45
        width: 45
        visible: isBusy
        running: visible

        TapHandler {
            enabled: isBusy && errorPopup.errorMessage.length > 0

            onTapped: {
                errorPopup.open();
            }
        }
    }

    //! Temp connection to go to the next page.
    Connections {
        target: deviceController
        enabled: root.visible && (isBusyCustomer || isBusyZip)

        function onCustomerInfoReady(error, isNeedRetry) {
            isBusyCustomer = isNeedRetry && error.length > 0;

            if (error.length > 0) {
                errorPopup.errorMessage = "Customer information is not ready, " + error;

                if (isNeedRetry) {
                    retryTimer.start();
                }

                if ((retryTimer.retryEmailCounter % 2 === 0) || !isNeedRetry) {
                    errorPopup.open();
                }

            }  else {
                retryTimer.retryEmailCounter = 0
                nextPage();
            }
        }

        function onZipCodeInfoReady(error, isNeedRetry) {
            isBusyZip = isNeedRetry && error.length > 0;

            if (error.length > 0) {
                errorPopup.errorMessage = "ZIP code information is not ready, " + error;

                if (isNeedRetry) {
                    retryTimer.start();
                }

                if ((retryTimer.retryZIPCounter % 2 === 0) || !isNeedRetry) {
                    errorPopup.open();
                }

            } else {
                retryTimer.retryZIPCounter = 0;

                // Get the customer information
                isBusyCustomer = true;
                deviceController.getEmailJobInformationManual();
            }
        }
    }


    Timer {
        id: retryTimer

        interval: 5000
        repeat: false
        running: false

        property int retryZIPCounter: 0
        property int retryEmailCounter: 0

        onTriggered: {
            if (isBusyCustomer) {
                retryEmailCounter++;
                deviceController.getEmailJobInformationManual();

            } else if (isBusyZip) {
                retryZIPCounter++;
                deviceController.getZipCodeJobInformationManual();

            } else {
                retryTimer.stop();
            }
        }
    }

    InitialFlowErrorPopup {
        id: errorPopup

        isBusy: isBusyCustomer|| isBusyZip
        deviceController: uiSession.deviceController

        onStopped: {
            isBusyCustomer = false;
            isBusyZip = false;
            retryTimer.stop();
        }

    }

    /* Functions
     * ****************************************************************************************/

    //! Go to CustomerDetailsPage
    function nextPage() {
        //! we prevent to go next page as one of calls is busy or has error
        if (isBusy)
            return;

        // Go to next page
        if (root.StackView.view) {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/InstallationTypePage.qml", {
                                          "uiSession": uiSession,
                                         "initialSetup": root.initialSetup
                                      });
        }
    }

}
