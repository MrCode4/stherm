import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ThermostatNamePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property bool initialSetup: false

    //! Busy due to get the Install operation
    property bool isBusy: false

    /* Object properties
     * ****************************************************************************************/
    title: "Thermostat Name"

    onVisibleChanged: {
        if (!visible) {
            retryTimer.stop();
            errorPopup.close();
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

    ColumnLayout {
        anchors.top: parent.top
        anchors.topMargin: 25
        anchors.horizontalCenter: parent.horizontalCenter

        width: parent.width * 0.95
        spacing: 4

        Label {
            text: "Thermostat name"
            font.pointSize: root.font.pointSize
        }

        TextField {
            id: nameTf

            Layout.fillWidth: true
            placeholderText: "Input the name"
            text: appModel?.thermostatName ?? ""
            validator: RegularExpressionValidator {
                regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
            }
            readOnly: isBusy

            onAccepted: {
                submitBtn.forceActiveFocus();
                submitBtn.clicked();
            }
        }

        Label {
            Layout.fillWidth: true

            width: parent.width
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
            font.pointSize: root.font.pointSize * 0.7
            text: "Please ask the customer for the thermostat name. " +
                  "The name is needed to differentiate between thermostats when using the Mobile app."
        }
    }

    BusyIndicator {
        anchors.right: submitBtn.left
        anchors.verticalCenter: submitBtn.verticalCenter

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

    Label {
        anchors.bottom: submitBtn.top
        anchors.bottomMargin: 10
        anchors.left: parent.left
        width: parent.width

        font.pointSize: Application.font.pointSize * 0.7
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment:Text.AlignBottom
        text: "Contact Nuve Support: (657) 626-4887 for issues."
    }

    //! Submit button
    ButtonInverted {
        id: submitBtn

        //! Has the initial flow been submitted?
        //! TODO: When thermostatName changed, the model should be submitted again.
        property bool submitted: false

        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10

        enabled: !isBusy
        text: "Submit"
        visible: !nameTf.activeFocus
        leftPadding: 25
        rightPadding: 25

        onClicked: {
            isBusy = true;

            appModel.thermostatName = nameTf.text;

            if (NetworkInterface.hasInternet) {
                retryTimer.triggered();
                submitBtn.submitted = true;

            } else {
                errorPopup.errorMessage = "No internet connection. Please check your internet connection.";
                errorPopup.open();
            }
        }
    }

    //! Temp connection to end busy
    Connections {
        target: deviceController.sync
        enabled: root.visible && isBusy

        function onInstalledSuccess() {
            isBusy = false;
            retryTimer.retryCounter = 0;
        }

        function onInstallFailed(err : string, needToRetry : bool) {
            isBusy = needToRetry;
            errorPopup.errorMessage = err;

            if (needToRetry) {
                retryTimer.start();
            }

            if (!needToRetry || (retryTimer.retryCounter % 2 === 0)) {
                errorPopup.open();
            }
        }

    }

    InitialFlowErrorPopup {
        id: errorPopup

        isBusy: root.isBusy
        deviceController: uiSession.deviceController

        onStopped: {
            root.isBusy = false;
            retryTimer.stop();
        }
    }

    Timer {
        id: retryTimer

        property int retryCounter: 0

        interval: 5000
        repeat: false
        running: false

        onTriggered: {
            if (initialSetup) {
                retryCounter++;
                deviceController.pushInitialSetupInformation();
            }
        }
    }

}
