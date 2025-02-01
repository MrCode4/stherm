import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WarrantyReplacementPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    property System system: deviceController?.system ?? null

    property Sync   sync:   deviceController?.sync   ?? null


    //! Busy due to warranty replacment operation
    property bool isBusy: false

    /* Object properties
     * ****************************************************************************************/
    title: "Warranty Replacement"
    titleHeadeingLevel: 4

    onVisibleChanged: {
        if (!visible) {
            tryTimer.stop();
        }
    }

    /* Children
     * ****************************************************************************************/

    //! Wifi status
    WifiButton {
        parent: root.header.contentItem
        visible: !deviceController.initialSetupNoWIFI && !NetworkInterface.hasInternet

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

    Label {
        id: warranryReplacementInfo

        anchors.top: parent.top
        anchors.topMargin: -5

        width: parent.width * 0.95
        wrapMode: Text.WordWrap
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignHCenter
        font.pointSize: root.font.pointSize * 0.67
        text: "To replace the damaged thermostat under warranty, " +
              "enter the serial number (S/N) of the damaged thermostat " +
              "in the 'Old S/N' box below. Then click 'Replace' to proceed."
    }

    ColumnLayout {
        anchors.top: warranryReplacementInfo.bottom
        anchors.topMargin: 5
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.9

        Label {
            text: "Old S/N"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: oldSNTf

            topPadding: 0
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            font.pointSize: root.font.pointSize * 0.8
            placeholderText: "Input the S/N of damaged thermostat"
            inputMethodHints: Qt.ImhPreferNumbers

            property string updateText: ""

            onTextChanged: {
                if (text.length > updateText.length && (text.match(/^\d{2}$/) || text.match(/^\d{2}-\d{3}$/))) {
                    text += "-";
                    updateText = text;

                } else if (text.length < updateText.length && updateText.endsWith("-")) {
                    // Update the updateText before text changed.
                    updateText = text.substring(0, text.length - 1);
                    text = updateText;

                } else {
                    updateText = text;
                }

                //! Serial number changed, abort the request.
                isBusy = false;
                tryTimer.stop();
            }

            validator: RegularExpressionValidator {
                regularExpression: /^\d{2}-\d{3}-\d{6}$/
            }
        }

        Label {
            Layout.fillWidth: true

            width: parent.width
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.67
            text: "Check the S/N (Serial Number) on the back of the thermostat (e.g., 01-224-001212)."
        }

        Item {
            id: spacer
            Layout.fillWidth: true
            height: 10
        }

        Label {
            text: "New S/N"
            font.pointSize: root.font.pointSize * 0.9
        }

        TextField {
            id: newSNTf

            Layout.fillWidth: true

            // Device with the new serial number
            text: system?.serialNumber ?? ""
            topPadding: 0
            font.pointSize: root.font.pointSize * 0.8
            Layout.preferredHeight: 50
            placeholderText: "New S/N"
            inputMethodHints: Qt.ImhPreferNumbers

            property string updateText: ""

            onTextChanged: {
                if (text.length > updateText.length && (text.match(/^\d{2}$/) || text.match(/^\d{2}-\d{3}$/))) {
                    text += "-";
                    updateText = text;

                } else if (text.length < updateText.length && updateText.endsWith("-")) {
                    // Update the updateText before text changed.
                    updateText = text.substring(0, text.length - 1);
                    text = updateText;

                } else {
                    updateText = text;
                }

                //! Serial number changed, abort the request.
                isBusy = false;
                tryTimer.stop();
            }

            validator: RegularExpressionValidator {
                regularExpression: /^\d{2}-\d{3}-\d{6}$/
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }

    BusyIndicator {
        anchors.right: replaceBtn.left
        anchors.verticalCenter: replaceBtn.verticalCenter

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

    //! Replace button
    ButtonInverted {
        id: replaceBtn

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        anchors.bottomMargin: 5

        text: "Replace"
        visible: !oldSNTf.activeFocus && !newSNTf.activeFocus
        enabled: oldSNTf.acceptableInput && newSNTf.acceptableInput && !isBusy
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            if (NetworkInterface.hasInternet) {
                isBusy = true;
                tryTimer.stop();
                tryTimer.triggered();

            } else {
                errorPopup.errorMessage = deviceController.deviceInternetError();
                errorPopup.open();
            }
        }
    }

    Timer {
        id: tryTimer

        property int retryCounter: 0

        interval: 5000
        repeat: false
        running: false

        onTriggered: {
            retryCounter++;
            sync?.warrantyReplacement(oldSNTf.text, newSNTf.text);
        }
    }

    CriticalErrorDiagnosticsPopup {
        id: errorPopup

        isBusy: root.isBusy
        deviceController: uiSession.deviceController

        onStopped: {
            root.isBusy = false;
            tryTimer.stop();
        }
    }

    Connections {
        target: sync

        // To avoid restart the aborted request.
        enabled: isBusy

        function onWarrantyReplacementFinished(success: bool, error: string, needToRetry: bool) {
            isBusy = !success && needToRetry;

            if (!success) {
                errorPopup.errorMessage = error;

                if (needToRetry) {
                    tryTimer.start();
                }

                if ((tryTimer.retryCounter % 2 === 0) || !needToRetry){
                    errorPopup.open();
                }
            }
        }
    }
}
