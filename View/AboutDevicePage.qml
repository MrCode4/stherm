import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as Template

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AboutDevicePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property I_DeviceController     deviceController: uiSession?.deviceController ?? null

    //! System, use in update notification
    property System                 system:           deviceController.deviceControllerCPP.system

    property int testCounter: 0

    property string appVesion: ""
    property bool showTestMode: false

    //! true: Restart device after all wifis forgotten in this page.
    property bool _restartDeviceAfterForgotWiFis: false

    /* Object properties
     * ****************************************************************************************/
    title: "Device Info"

    Component.onCompleted: {
        const versionArray = Application.version.split('.')
        const versionArrayMain = versionArray.splice(0, 3)
        appVesion = versionArrayMain.join('.')
    }

    /* Children
     * ****************************************************************************************/

    Connections {
        target: system
        function onLogPrepared(isSuccess){
            logBusyPop.close()
            if (isSuccess){
                uiSession.popUps.initSendingLogProgress();
            } else {
                logBusyPop.message = "File generation failed."
                logBusyPop.open()
            }
        }
    }

    ListView {
        id: _infoLv

        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: root.contentItem.y
            parent: root
            height: root.contentItem.height - 30
        }

        anchors.fill: parent
        anchors.rightMargin: 10
        anchors.bottomMargin: 5

        clip: true
        model: [
            { "key": "Model",               "value": "Nuve - Samo" },
            { "key": "FCC ID",              "value": "2BBXVSAMOV1" },
            { "key": "Contians FCC ID",     "value": "VPYLB1DX" },
            { "key": "IC",                  "value": "LBWA1KL1FX-875" },
            { "key": "Serial No",           "value": system.serialNumber },
            { "key": "Software version",    "value": appVesion },
            { "key": "Hardware version",    "value": "01" },
            { "key": "Custom Name",         "value": "Living Room" },
            { "key": "URL",                 "value": '<a href="nuvehome.com" style="text-decoration:none;color:#44A0FF;">nuvehome.com</a>' },
            { "key": "E-mail",              "value": '<a href="support@nuvehome.com" style="text-decoration:none;color:#44A0FF;">support@nuvehome.com</link>' },
            { "key": "IPv4 Address",        "value": NetworkInterface.ipv4Address }
        ]
        delegate: Item {
            width: ListView.view.width
            height: visible ? Style.delegateHeight * 0.8 : 0
            visible: modelData?.visible ?? true

            RowLayout {
                id: textContent
                spacing: 16
                visible: (modelData?.type !== "button") ?? true
                anchors.fill: parent

                Label {
                    Layout.preferredWidth: _fontMetrics.boundingRect("Hardware version :").width + leftPadding + rightPadding
                    font.bold: true
                    text: modelData.key + ":"
                }

                Label {
                    Layout.fillWidth: true
                    font.pointSize: Application.font.pointSize * 0.9
                    textFormat: "RichText"
                    horizontalAlignment: "AlignRight"
                    text: modelData.value

                }
            }

            //! to start test mode Easter Egg
            TapHandler {
                enabled: textContent.visible
                onTapped: {
                    if (index === 1) {
                        root.testCounter++;
                        if (root.testCounter === 10) {
                            root.testCounter = 0;
                            if (root.StackView.view) {
                                uiSession.uiTestMode = true;
                                root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {
                                                             "uiSession": uiSession
                                                         })
                            }
                        }
                    }
                }
            }
        }

        footer: ColumnLayout {
            spacing: 6
            anchors.horizontalCenter: parent.horizontalCenter

            Repeater {
                model: [
                    {
                        text: qsTr("Send Log"), action: () => {
                            if (NetworkInterface.hasInternet) {
                                if (system.isBusylogSender()) {
                                    // Log sender is busy, open the progress bar.
                                    uiSession.popUps.showSendingLogProgress();

                                } else {
                                    // Prepare the log
                                    logBusyPop.message = "Preparing log, \nplease wait ..."
                                    logBusyPop.open();
                                }
                            }

                            else system.alert("No Internet, Please check your connection before sending log.")
                        },
                        buddies: [
                            {
                                text: qsTr("Update NRF"), visible: system.testMode, action: () => {
                                    deviceController.deviceControllerCPP.updateNRFFirmware();
                                }
                            }
                        ]
                    },
                    {
                        text: qsTr("Test Mode"), visible: root.showTestMode, action: () => {
                            if (root.StackView.view) {
                                deviceController.postWarrantyReplacementFinsihed();
                                uiSession.uiTestMode = true;
                                root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {"uiSession": uiSession});
                            }
                        }
                    },
                    {
                        text: qsTr("Restart Device"), action: () => {
                            root.showCountDownPopUpForRestartDevice()
                        },
                        buddies: [
                            {
                                text: qsTr("Restart App"), visible: root.showTestMode || system.testMode, action: () => {
                                    root.showCountDownPopUpForRestartApp()
                                }
                            }
                        ]

                    },
                    {
                        text: qsTr("Manage Endpoint"), visible: root.showTestMode  || system.testMode, action: () => {
                            if (root.StackView.view) {
                                root.StackView.view.push("qrc:/Stherm/View/Menu/ManageEndpoint.qml", {"uiSession": uiSession});
                            }
                        }
                    },
                    {
                        text: qsTr("Test Config"), visible: system.testMode, action: () => {
                            if (root.StackView.view) {
                                root.StackView.view.push("qrc:/Stherm/View/Test/TestConfigPage.qml", {"uiSession": uiSession});
                            }
                        }
                    },
                    {
                        text: qsTr("No Wi-Fi Config"), visible: system.testMode, action: () => {
                            if (root.StackView.view) {
                                root.StackView.view.push("qrc:/Stherm/View/Menu/LimitedModeRemainigTimePage.qml", {"uiSession": uiSession});
                            }
                        }
                    },
                    {
                        text: qsTr("Contractor Info Test"), visible: deviceController.initialSetup, action: () => {
                            if (root.StackView.view) {
                                root.StackView.view.push("qrc:/Stherm/View/SystemUpdatePage.qml", {
                                                             "uiSession": uiSession,
                                                             "contractorFlow": true
                                                         });
                            }
                        }
                    },
                    {
                        text: qsTr("Forget Device"), visible: deviceController.initialSetup, action: () => {
                            root.showCountDownPopUpForForgetDevice()
                        }
                    },
                    {
                        text: qsTr("Exit"), visible: system.testMode, action: () => {
                            root.showCountDownPopUpForStopDevice()
                        },
                        buddies: [
                            {
                                //! Forget device is visible in another row on initial setup
                                text: qsTr("Forget Device"), visible: !deviceController.initialSetup, action: () => {
                                    root.showCountDownPopUpForForgetDevice()
                                }
                            }
                        ]
                    },
                    {
                        text: qsTr("Reset Factory"), visible: system.testMode, action: () => {
                            resetFactoryPopUp.open();
                        }
                    }
                ]

                RowLayout {
                    spacing: 16
                    Layout.alignment: Qt.AlignHCenter
                    visible: modelData.visible ?? true

                    ButtonInverted {
                        leftPadding: 8
                        rightPadding: 8
                        text: modelData.text

                        onClicked: {
                            if (modelData.action instanceof Function) {
                                modelData.action()
                            }
                        }
                    }

                    Repeater {
                        model: modelData.buddies
                        ButtonInverted {
                            leftPadding: 8
                            rightPadding: 8
                            text: modelData.text
                            visible: modelData.visible ?? true

                            onClicked: {
                                if (modelData.action instanceof Function) {
                                    modelData.action()
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    FontMetrics {
        id: _fontMetrics
    }

    Popup {
        id: logBusyPop
        property string message: ""

        parent: Template.Overlay.overlay
        closePolicy: Popup.NoAutoClose
        width: Math.max(implicitWidth, parent.width * 0.5)
        height: parent.height * 0.5
        anchors.centerIn: parent
        modal: true

        onOpened: {
            //! Call sendLog()
            system.sendLog();
        }

        contentItem: Label {
            text: logBusyPop.message
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
            lineHeight: 1.4
        }
    }

    ResetFactoryPopUp {
        id: resetFactoryPopUp

        onReset: {
            busyPopUp.open();
            deviceController.sync.resetFactory();
            close();
        }
    }

    BusyPopUp {
        id: busyPopUp
    }

    AlertNotifPopup {
        id: alertNotifPopup

        message: Message {
            message: qsTr("Failed to remove the device from the server. Please try again.")
            type: Message.Alert
        }
    }

    Connections {
        target: deviceController?.sync ?? null

        function onResetFactoryFinished(ok: bool, message: string) {
            busyPopUp.close()
            if(ok === true) {
                root.showCountDownPopUpForResetFactory()
            } else {
                alertNotifPopup.open();
            }
        }
    }

    Connections {
        target: NetworkInterface

        enabled: root.visible

        function onAllWiFiNetworksForgotten() {
            if (root._restartDeviceAfterForgotWiFis)
                deviceController.resetDeviceToFactory();
        }
    }

    function showCountDownPopUpForForgetDevice() {
        uiSession.popUps.showCountDownPopUp(
                    qsTr("Forget Device"),
                    qsTr("Restarting Device..."),
                    true,
                    function () {
                        deviceController.forgetDevice();

                        if (system) {
                            system.rebootDevice();
                        }
                    });
    }

    function showCountDownPopUpForResetFactory() {
        root._restartDeviceAfterForgotWiFis = true;
        uiSession.popUps.showCountDownPopUp(
                    qsTr("Reset Device to Factory Setting"),
                    qsTr("Restarting Device..."),
                    false,
                    function () {
                        NetworkInterface.forgetAllWifis();
                    });
    }

    function showCountDownPopUpForRestartApp() {
        uiSession.popUps.showCountDownPopUp(
                    qsTr("Restart App"),
                    qsTr("Restarting App..."),
                    true,
                    function () {
                        if (system) {
                            system.systemCtlRestartApp();
                        }
                    });
    }

    function showCountDownPopUpForRestartDevice() {
        uiSession.popUps.showCountDownPopUp(
                    qsTr("Restart Device"),
                    qsTr("Restarting Device..."),
                    true,
                    function () {
                        if (system) {
                            system.rebootDevice();
                        }
                    });
    }

    function showCountDownPopUpForStopDevice() {
        uiSession.popUps.showCountDownPopUp(
                    qsTr("Stop Device"),
                    qsTr("Stopping Device..."),
                    true,
                    function () {
                        if (system) {
                            system.stopDevice();
                        }
                    });
    }
}
