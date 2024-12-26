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
                                uiSession.uiTestMode = true;
                                root.StackView.view.push("qrc:/Stherm/View/Test/VersionInformationPage.qml", {"uiSession": uiSession});
                            }
                        }
                    },
                    {
                        text: qsTr("Restart Device"), action: () => {
                            rebootPopup.withForget = false;
                            rebootPopup.open();
                        },
                        buddies: [
                            {
                                text: qsTr("Restart App"), visible: root.showTestMode || system.testMode, action: () => {
                                    restartAppPopup.open();
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
                            rebootPopup.withForget = true;
                            rebootPopup.open();
                        }
                    },
                    {
                        text: qsTr("Exit"), visible: system.testMode, action: () => {
                            exitPopup.open();
                        },
                        buddies: [
                            {
                                //! Forget device is visible in another row on initial setup
                                text: qsTr("Forget Device"), visible: !deviceController.initialSetup, action: () => {
                                    rebootPopup.withForget = true;
                                    rebootPopup.open();
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

    //! Reboot popup with count down timer to send reboot request to system
    RebootDevicePopup {
        id: rebootPopup

        //! Enable forget device in this popup
        property bool withForget: false

        title : withForget ? "   Forget Device   " :  " Restart Device   "
        anchors.centerIn: Template.Overlay.overlay

        onStartAction: {
            if (withForget) {
                deviceController.forgetDevice();
            }

            if (system) {
                system.rebootDevice();
            }
        }
    }

    //! Restart popup with count down timer to send restart request to system
    RebootDevicePopup {
        id: restartAppPopup

        title: "   Restart App   "
        infoText: "Restarting App..."
        anchors.centerIn: Template.Overlay.overlay

        onStartAction: {
            if (system) {
                system.systemCtlRestartApp();
            }
        }
    }

    property RebootDevicePopup resetFactoryPopup: null
    Component {
        id: resetFactoryPopupComponent

        RebootDevicePopup {
            title : "   Forget Device   "
            anchors.centerIn: Template.Overlay.overlay
            cancelEnable: false

            onStartAction: {
                deviceController.resetDeviceToFactory();

                if (system) {
                    system.rebootDevice();
                }
            }

            onClosed: {
                destroy(this);
                resetFactoryPopup = null;
            }
        }
    }

    //! Exit popup with count down timer to send exit request to system
    ResetDevicePopup {
        id: exitPopup
        system: root.system
        anchors.centerIn: Template.Overlay.overlay
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
            busyPopUp.open()
            deviceController.sync.resetFactory()
            resetFactoryPopUp.close()
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

        function onResetFactorySucceeded() {
            NetworkInterface.forgetAllWifis();
            busyPopUp.close()

            showResetFactoryPopup();
        }

        function onResetFactoryFailed() {
            alertNotifPopup.open()
            busyPopUp.close()
        }
    }

    //! Show the reset factory popup.
    function showResetFactoryPopup() {
        if (!resetFactoryPopup) {
            resetFactoryPopup = resetFactoryPopupComponent.createObject(root);
        }

        resetFactoryPopup.open();
    }
}
