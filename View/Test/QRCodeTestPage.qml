import QtQuick

import Ronia
import Stherm
import QtQuick.Templates as Template

/*! ***********************************************************************************************
 * QRCodeTestPage: Generate a QR code based on {"hv":"01","uid":"000000000000000"} string
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property Declaration
     * ****************************************************************************************/
    property System system: deviceController.deviceControllerCPP.system

    property bool   startSNCheck: false

    property int    retrySN:      0

    property bool   backButtonWasVisible: backButtonVisible

    /* Object properties
     * ****************************************************************************************/

    title: "System  QR Code "
    useSimpleStackView: true

    /* Children
     * ****************************************************************************************/

    //! Reboot popup with count down timer to send reboot request to system
    CountDownPopup {
        id: forgetWifisPopup

        anchors.centerIn: Template.Overlay.overlay

        title: "   Restart Device   "
        actionText: "Serial number is ready.\nRestarting Device..."

        onStartAction: {
            // Forget Wifis
            actionText = "Serial number is ready.\nForgetting Wi-Fis..."
            NetworkInterface.forgetAllWifis();
        }
    }

    InfoPopup {
        id: infoPopup
        message: "QR Code"
        detailMessage: "Serial number is not valid.<br>Please try again."
        visible: false
    }

    //! To show test and publish tests results.
    InfoPopup {
        id: testInfoPopup

        visible: false
        message: "Test results"

        onAccepted: {
            if (system.serialNumber.length > 0) {
                finishButton.enabled = true;
                forgetWifisPopup.open();
            } else {
                infoPopup.open();
            }
        }
    }

    //! ConfirmPopup to ask: "Have you printed the SN label?"
    //! The testFinished function is executed.
    //! If the serial number (SN) is available, a reboot popup is displayed.
    //! Otherwise, an informational popup is presented.
    ConfirmPopup {
        id: printConfirmPopup

        message: "Print SN Label"
        detailMessage: "Have you printed out the SN label?"
        visible: false

        onAccepted: {
            //! To ensure test finished called at least once
            deviceController.deviceControllerCPP.testFinished();

            console.log("QRCodeTestPage, finished test, serial number:", system.serialNumber, system.serialNumber.length)

            testInfoPopup.pending = true

            testInfoPopup.detailMessage = (deviceController.deviceControllerCPP.isTestsPassed() ? "All tests are passed." : "All tests are not passed.") +
                    "<br>" + "Test results are being published.";
            console.log("print label accepted:", testInfoPopup.detailMessage);
            testInfoPopup.open();
        }
    }

    //! Finish button
    ToolButton {
        id: finishButton
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            var sn = system.serialNumber;

            console.log("QRCodeTestPage, checking for serial number ready:", sn, sn.length)

            //! to prevent excess tapping by user when snChecker timer already retrying
            if (sn.length === 0 && !startSNCheck) {
                // restore backbutton after busy indicator done
                backButtonWasVisible = backButtonVisible;
                backButtonVisible = false;
                // to ensure busy indicator starts
                retrySN++;
                //! Try to check serial number
                snChecker.triggered();
                startSNCheck = true;
            }

            printConfirmPopup.open();
        }

        //! BusyIndicator for Fetching SN running status.
        BusyIndicator {
            anchors {
                horizontalCenter: parent.horizontalCenter
                top: parent.bottom
                topMargin: -10
            }

            width: parent.width
            visible: running
            running: system.serialNumber.length === 0 && retrySN > 0
            onRunningChanged: {
                if (!running) {
                    backButtonVisible = backButtonWasVisible;
                }
            }

            Label {
                anchors.centerIn: parent
                text: retrySN
            }
        }
    }


    //! User guide link QR code
    Image {
        id: qrCodeImage

        anchors.centerIn: parent

        property string url: "{\"hv\":\"01\",\"uid\":\"" + system.systemUID +"\"}"

        width: parent.width * 0.75
        height: width
        source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg(qrCodeImage.url, Style.foreground)}`
        sourceSize.width: width
        sourceSize.height: height
        fillMode: Image.PreserveAspectFit
    }

    InfoToolButton {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        visible: true

        onClicked: {
            if (root.StackView.view) root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {"uiSession": Qt.binding(() => uiSession)});
        }
    }

    //! The timer will be start when
    //!   SerialNumber is ready so the user can print sn
    //!   All popups hidden.
    //! 5-second timer starts to ensure the user has sufficient time to see and potentially scan the QR code.
    //! The timer open printConfirmPopup
    Timer {
        id: printCheckTimer

        interval: 5000
        running: root.visible && system.serialNumber.length > 0 &&
                 !forgetWifisPopup.visible && !printConfirmPopup.visible && !testInfoPopup.visible

        repeat: true

        onTriggered: {
            // JUST check, always true in this scope
            if (system.serialNumber.length > 0) {
                infoPopup.close();
                printConfirmPopup.open();
            }
        }
    }

    property Timer snChecker: Timer {
        repeat: true
        running: startSNCheck && system.serialNumber.length === 0
        interval: 10000

        onTriggered: {
            var uid = deviceController.deviceControllerCPP.deviceAPI.uid;
            system.fetchSerialNumber(uid, false);
            retrySN++;
        }

    }

    Connections {
        target: system

        function onTestPublishFinished(msg: string) {
            testInfoPopup.detailMessage = (deviceController.deviceControllerCPP.isTestsPassed() ? "All tests are passed." : "All tests are not passed.") +
                    "<br>" +
                    (msg.length > 0 ? ("Test results are not published.<br> " + msg) : "Test results are published.");

            console.log("onTestPublishFinished:", testInfoPopup.detailMessage);

            testInfoPopup.pending = false;

            // no need but keept anyway
            testInfoPopup.open();
        }
    }

    Connections {
        target: NetworkInterface

        enabled: root.visible

        function onAllWiFiNetworksForgotten() {
            forgetWifisPopup.actionText = "Serial number is ready.\nRestarting Device..."
            system.rebootDevice();
        }
    }
}
