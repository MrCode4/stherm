import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * DeviceLocationPage provides ui for choosing device location
 * ***********************************************************************************************/
InitialSetupBasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property string deviceLocation: appModel?.deviceLocation ?? ""

    //! Busy due to get the Install operation
    property bool isBusy: false

    /* Object properties
     * ****************************************************************************************/
    title: "Device Location"

    showWifiButton: true

    onVisibleChanged: {
        if (!visible) {
            retryTimer.stop();
            errorPopup.close();
        }
    }

    /* Children
     * ****************************************************************************************/

    Flickable {
        id: itemsFlickable

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 16
        }

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: infoLabel.top
        anchors.bottomMargin: 10
        anchors.rightMargin: 10

        clip: true
        enabled: !isBusy
        boundsBehavior: Flickable.StopAtBounds
        contentHeight: _contentLay.implicitHeight
        contentWidth: width

        ColumnLayout {
            id: _contentLay

            anchors.centerIn: parent
            width: parent.width * 0.65

            Repeater {
                model: AppSpec.deviceLoacations[appModel?.residenceType ?? AppSpec.Unknown]

                delegate: Button {
                    Layout.fillWidth: true

                    topInset: 2
                    bottomInset: 2
                    text: modelData
                    autoExclusive: true
                    checked: deviceLocation === text

                    onClicked: {
                        appModel.whereInstalled = index;
                        appModel.deviceLocation = String(modelData);
                        appModel.thermostatName = String(modelData);

                        nextPage();
                    }
                }
            }
        }
    }


    ContactNuveSupportLabel {
        id: infoLabel

        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.left: parent.left
        width: parent.width
    }

    BusyIndicator {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

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
            retryCounter++;
            deviceController.pushInitialSetupInformation();
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

    /* Functions
     * ****************************************************************************************/

    function nextPage() {
        retryTimer.stop();

        if (root.StackView.view && appModel.deviceLocation === "Custom") {
            root.StackView.view.push("qrc:/Stherm/View/SystemSetup/ThermostatNamePage.qml", {
                                         "uiSession": Qt.binding(() => uiSession),
                                         "initialSetup":  root.initialSetup
                                     });
        } else {
            if (NetworkInterface.hasInternet) {
                isBusy = true;
                retryTimer.retryCounter = 0;
                retryTimer.triggered();

            } else {
                errorPopup.errorMessage = deviceController.deviceInternetError();
                errorPopup.open();
            }
        }
    }
}
