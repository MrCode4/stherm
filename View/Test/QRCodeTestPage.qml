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

    /* Object properties
     * ****************************************************************************************/

    title: "System  QR Code "
    useSimpleStackView: true

    /* Children
     * ****************************************************************************************/

    //! Reboot popup with count down timer to send reboot request to system
    RebootDevicePopup {
        id: rebootPopup

        infoText: "Serial number is ready.\nRestarting Device..."
        anchors.centerIn: Template.Overlay.overlay

        onStartAction: {
            if (system) {
                system.rebootDevice();
            }
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
                rebootPopup.open();
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

            if (system.serialNumber.length === 0) {
                infoPopup.open();

                //! Retry to check serial number
                startSNCheck = true;
            }
        }
    }

    //! Finish button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            var sn = system.serialNumber;

            console.log("QRCodeTestPage, checking for serial number ready:", sn, sn.length)

            if (sn.length === 0) {
                //! Retrieve Serial Number (SN) using UID and await response
                var uid = deviceController.deviceControllerCPP.deviceAPI.uid;
                system.fetchSerialNumber(uid, false);
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

        onClicked: gotoPage("qrc:/Stherm/View/AboutDevicePage.qml", {"uiSession": Qt.binding(() => uiSession), 'useSimpleStackView': true});
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
                 !rebootPopup.visible && !printConfirmPopup.visible

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
            retrySN ++;
        }

    }

    Connections {

        target: system

        function onTestPublishFinished(msg: string) {
            testInfoPopup.detailMessage = (deviceController.deviceControllerCPP.isTestsPassed() ? "All tests are passed" : "All tests are not passed") +
                    "<br>" +
                    (msg.length > 0 ? "Tests are not published." : "Tests are published.");

            console.log(testInfoPopup.detailMessage);

            testInfoPopup.open();
        }
    }
}
