import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiPage provides a ui to connect to a Wi-Fi network
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    readonly property var sortedWifis: {
        var wifis = Array.from(NetworkInterface.wifis);
        return wifis.sort((a, b) => b.strength - a.strength).filter((element, index) => element.strength > -1);
    }

    readonly property var savedNonInRangeWifis: {
        var wifis = Array.from(NetworkInterface.wifis);
        return wifis.filter((element, index) => element.strength < 0 && element.ssid !== "");
    }

    property System system: deviceController.deviceControllerCPP.system

    //! Use deviceController to bind better.
    property bool initialSetup: deviceController.initialSetup

    property bool initialSetupNoWIFI: deviceController.initialSetupNoWIFI
    property bool openFromNoWiFiInstallation: initialSetupNoWIFI

    property bool initialSetupReady : initialSetup && system.serialNumber.length > 0 &&
                                      deviceController.checkedSWUpdate && NetworkInterface.connectedWifi

    //! To conditionally display and hide/show the "Next" button and disable/enable the next timer based on specific scenarios,
    //! such as during initial device setup in warranty replacment page.
    property bool nextButtonEnabled: initialSetup

    readonly property bool selectConnectedWifi: _wifisRepeater.currentIndex === -1

    /* Object properties
     * ****************************************************************************************/
    title: "Wi-Fi Settings"
    topPadding: bottomPadding + 12

    /* Children
     * ****************************************************************************************/

    //! Once the network connection is established, the System Types page should automatically open,
    Timer {
        id: nextPageTimer

        property bool once : false

        repeat: false
        running: !once && root.visible && initialSetupReady &&  nextButtonEnabled
        interval: 10000
        onTriggered: nextPage()
    }

    RowLayout {
        parent: root.header.contentItem

        Switch {
            id: _wifiOnOffSw

            enabled: false && !NetworkInterface.busyRefreshing
            visible: false
            checked: NetworkInterface.deviceIsOn
            onToggled: {
                if (checked) {
                    NetworkInterface.turnOn();
                } else {
                    NetworkInterface.turnOff();
                }
            }
        }

        //! Referesh button and running BusyIndicator
        Item {
            Layout.alignment: Qt.AlignCenter
            implicitWidth: Material.touchTarget
            implicitHeight: Material.touchTarget

            ToolButton {
                id: _refereshBtn
                anchors.centerIn: parent
                visible: !NetworkInterface.busyRefreshing
                contentItem: RoniaTextIcon {
                    text: "\uf2f9" //! rotate-right
                }

                onClicked: {
                    // Deselect the wifis.
                    _wifisRepeater.currentIndex = -2;

                    //! Force refresh
                    if (NetworkInterface.deviceIsOn && !NetworkInterface.busyRefreshing) {
                        NetworkInterface.refereshWifis(true);
                    }
                }
            }

            //! BusyIndicator for NetworkInterface running status
            BusyIndicator {
                anchors {
                    fill: parent
                    margins: 4
                }
                visible: running
                running: NetworkInterface.busyRefreshing
            }

            //! BusyIndicator for Fetching SN running status in first run flow
            BusyIndicator {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: parent.bottom
                    topMargin: -10
                }

                width: parent.width
                visible: running
                running: system.serialNumber.length === 0 && deviceController.checkSNTryCount > 0

                Label {
                    anchors.centerIn: parent
                    text: deviceController.checkSNTryCount
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Label {
            visible: Boolean(NetworkInterface.connectedWifi)
            opacity: 0.7
            font.pointSize: Application.font.pointSize * 0.8
            text: "Current Network"
        }

        WifiDelegate {
            id: _connectedWifiDelegate

            Layout.fillWidth: true
            Layout.bottomMargin: 16
            visible: wifi

            wifi: NetworkInterface.connectedWifi
            isWPA3: root.isSecuredByWPA3(wifi?.security ?? "")

            delegateIndex: -1
            isSelected: selectConnectedWifi

            onDisconnectClicked: {
                if (NetworkInterface.busy) {
                    return;
                }

                busyProcessPopup.isForgetting = false;
                busyProcessPopup.open();
                NetworkInterface.disconnectWifi(wifi);
                _wifisRepeater.currentIndex = -2;
            }

            onClicked: {
                if (selectConnectedWifi)
                    _wifisRepeater.currentIndex = -2;
                else
                    _wifisRepeater.currentIndex = -1;
            }
        }

        //! Available networks Label
        Label {
            id: _availLbl
            visible: _wifisRepeater.model.length > 0
            opacity: 0.7
            font.pointSize: Application.font.pointSize * 0.8
            text: "Available Networks"
        }

        //! Highlight Rectangle
        Rectangle {
            id: _hightlightRect

            readonly property alias wifiDelegate: _wifisRepeater.currentItem

            parent: wifiDelegate
            visible: wifiDelegate
            anchors.fill: parent ? parent : undefined
            color: Style.listHighlightColor
        }

        //! Wifis ListView
        Flickable {
            id: _wifisFlick

            ScrollIndicator.vertical: ScrollIndicator { }

            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            contentWidth: width
            contentHeight: _wifisCol.implicitHeight

            ColumnLayout {
                id: _wifisCol
                width: parent.width
                spacing: 8

                Repeater {
                    id: _wifisRepeater

                    property int currentIndex: -2 //! -1 means _connectedWifiDelegate is current item
                    readonly property WifiDelegate currentItem: (currentIndex > -2 && currentIndex < count)
                                                                ? selectConnectedWifi ? _connectedWifiDelegate
                                                                                      : itemAt(currentIndex)
                    : null

                    model: NetworkInterface.connectedWifi
                           ? sortedWifis.filter((element, index) => element !== NetworkInterface.connectedWifi)
                           : sortedWifis
                    delegate: WifiDelegate {
                        id: _wifiDelegate

                        required property var modelData
                        required property int index

                        Layout.fillWidth: true

                        wifi: (modelData instanceof WifiInfo) ? modelData : null
                        isWPA3: root.isSecuredByWPA3(wifi?.security ?? "")

                        delegateIndex: index
                        isSelected: (_wifisRepeater.currentItem?.wifi === wifi) ?? false

                        onClicked: {
                            if (_wifisRepeater.currentIndex === index)
                                _wifisRepeater.currentIndex = -2;
                            else
                                _wifisRepeater.currentIndex = index;
                        }

                        onConnectClicked: {
                            if (NetworkInterface.busy) {
                                return;
                            }

                            if ((root.isSecuredByWPA3(wifi.security) === true) && (NetworkInterface.doesDeviceSupportWPA3 === false)) {
                                 uiSession.popupLayout.displayPopUp(wpa3WifiAlert)
                                return
                            }

                            console.log("connecting to wifi: ", wifi.ssid, ", security: ", wifi.security)

                            //! Check if we need user prompt for password, i.e., access point is open or is saved and has a profile.
                            if (wifi.security === "" || NetworkInterface.isWifiSaved(wifi)) {
                                NetworkInterface.connectWifi(wifi, "");
                            } else {
                                var minPasswordLength = (wifi.security === "--" || wifi.security === "" ? 0 : 8)

                                //! Open connect page
                                if (root.StackView.view && _wifisRepeater.currentItem) {
                                    //! Note: it's better to stop wifi refreshing to prevent any deleted
                                    //! object access issues
                                    root.StackView.view.push("qrc:/Stherm/View/Wifi/WifiConnectPage.qml", {
                                                                 "uiSession": uiSession,
                                                                 "wifi": _wifisRepeater.currentItem.wifi,
                                                                 "minPasswordLength": minPasswordLength,
                                                             })
                                }
                            }

                            _wifisRepeater.currentIndex = -2;
                        }

                        onForgetClicked: {
                            if (uiSession) {
                                //! Ask for forgeting this wifi
                                _forgetDlg.wifiToForget = wifi;
                                uiSession.popupLayout.displayPopUp(_forgetDlg, true);
                            }
                        }
                    }
                }

                //! Saved networks Label
                Label {
                    visible: savedNonInRangeWifis.length > 0
                    opacity: 0.7
                    font.pointSize: Application.font.pointSize * 0.8
                    text: "Saved Networks"
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Repeater {
                        model: savedNonInRangeWifis
                        delegate: WifiDelegate {
                            required property var modelData
                            required property int index

                            Layout.fillWidth: true

                            wifiInRange: false;
                            isSelected: false
                            focus: false
                            focusPolicy: Qt.NoFocus
                            hoverEnabled: false
                            wifi: (modelData instanceof WifiInfo) ? modelData : null

                            onForgetClicked: {
                                if (NetworkInterface.busy) {
                                    return;
                                }

                                if (uiSession) {
                                    //! Ask for forgeting this wifi
                                    _forgetDlg.wifiToForget = wifi;
                                    uiSession.popupLayout.displayPopUp(_forgetDlg, true);
                                }
                            }
                        }
                    }
                }
            }
        }

        ContactNuveSupportLabel {
            Layout.preferredHeight: 35
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: infoButton.visible
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: Style.button.buttonHeight

            //! Manual button
            ButtonInverted {
                anchors.left: parent.left
                anchors.leftMargin: 8
                text: _wifisRepeater.currentItem?.wifi?.connected ? "Forget" : "Manual"
                onClicked: {
                    if (NetworkInterface.busy) {
                        return;
                    }

                    if (text === "Manual") {
                        if (root.StackView.view) {
                            root.StackView.view.push("qrc:/Stherm/View/Wifi/WifiManualConnectPage.qml");
                        }
                    } else {
                        if (uiSession) {
                            //! Ask for forgeting this wifi
                            _forgetDlg.wifiToForget = _wifisRepeater.currentItem?.wifi;
                            uiSession.popupLayout.displayPopUp(_forgetDlg, true);
                        }
                    }
                }
            }

            InfoToolButton {
                id: infoButton
                anchors.centerIn: parent

                visible: initialSetup

                onClicked: {
                    root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession)
                                             })

                }
            }

            //! Next button
            ButtonInverted {
                anchors.right: parent.right
                anchors.rightMargin: 8

                visible: nextButtonEnabled && !skipButton.visible
                enabled: initialSetupReady
                text: "Next"

                onClicked: {
                    nextPage();
                }
            }

            //! Skip, No wifi installation
            ButtonInverted {
                id: skipButton

                anchors.right: parent.right
                anchors.rightMargin: 8

                visible: root.initialSetup && !root.openFromNoWiFiInstallation && !NetworkInterface.connectedWifi &&
                         (deviceController.limitedModeRemainigTime > 0)
                text: "  Skip  "

                onClicked: {
                    uiSession.popUps.showSkipWIFIConnectionPopup();
                }
            }
        }
    }

    //! Forget wifi dialog
    ForgetWifiDialog {
        property WifiInfo   wifiToForget

        id: _forgetDlg
        wifiSsid: wifiToForget?.ssid ?? ""
        onAccepted: {
            if (wifiToForget) {
                busyProcessPopup.isForgetting = true;
                busyProcessPopup.open();

                //! Forget requested wifi
                NetworkInterface.forgetWifi(wifiToForget);
                wifiToForget = null;
            }
        }
    }

    //! Error Drawer
    Drawer {
        id: _errorDrawer

        Material.background: AppStyle.primaryRed
        width: parent.width
        height: Math.min(_errorMessageLbl.implicitHeight + topPadding + bottomPadding, parent.height / 2.8)
        edge: "BottomEdge"
        interactive: false
        closePolicy: Popup.NoAutoClose
        modal: false
        dim: false
        topPadding: 16
        bottomPadding: 16
        leftPadding: 8
        rightPadding: 8

        Label {
            id: _errorMessageLbl
            anchors.fill: parent
            textFormat: "MarkdownText"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
            wrapMode: "WordWrap"
            clip: true
        }

        Timer {
            interval: 4000 //! Drawer time out
            running: _errorDrawer.opened
            repeat: false
            onTriggered: _errorDrawer.close()
        }
    }

    //! Wifi and Internet connection alerts
    AlertNotifPopup {
        id: wpa3WifiAlert
        objectName: "WPA3Popup"

        uiSession: root.uiSession

        message: Message {
            title: "Network Not Supported"
            type: Message.Type.Alert
            message: "The selected Wi-Fi network uses WPA3 encryption, which is not supported by thermostat. Please choose a different network to connect."
        }
    }

    BusyPopUp {
        id: busyProcessPopup

        property bool isForgetting: false

        title: isForgetting ? "Forgetting..." : "Disconnecting..."
    }

    Connections {
        target: NetworkInterface

        enabled: root.visible

        function onWifiForgotten(wifi: WifiInfo) {
            console.log(wifi?.ssid, " forgotten.")
            busyProcessPopup.close();
        }

        function onConnectedWifiChanged() {

            // The popup will close when the connected Wi-Fi changed.
            if(busyProcessPopup.isForgetting == false)
                busyProcessPopup.close();

            // To address the issue of users reconnecting to Wi-Fi after navigating back from other pages.
            if (NetworkInterface.connectedWifi && !openFromNoWiFiInstallation)
                deviceController.setInitialSetupNoWIFI(false);
        }

        function onIncorrectWifiPassword(wifi: WifiInfo)
        {
            console.log('incorrect pass for: ', wifi.ssid);

            // TODO: manage push
            //! Incorrect password entered
            if (root.StackView.view && root.StackView.view.busy === false && root.StackView.view.currentItem === root) {
                var minPasswordLength = (wifi.security === "--" || wifi.security === "" ? 0 : 8)

                //! Note: it's better to stop wifi refreshing to prevent any deleted
                //! object access issues
                root.StackView.view.push("qrc:/Stherm/View/Wifi/WifiConnectPage.qml", {
                                             "uiSession": uiSession,
                                             "wifi": wifi,
                                             "minPasswordLength": minPasswordLength,
                                         })
            }
        }
    }

    Connections {
        target: uiSession
        enabled: root.visible && initialSetupNoWIFI

        function onGoToInitialSetupNoWIFIMode() {
            if (root.visible && initialSetupNoWIFI) {
                if (root.StackView.view) {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                             });
                }
            }
        }
    }

    onSortedWifisChanged: _wifisRepeater.currentIndexChanged();

    /* Functions
     * ****************************************************************************************/

    //! Called when initial setup is true
    function nextPage() {
        if (root.StackView.view) {
            nextPageTimer.once = true;
            if (system.serialNumber.length > 0) {

                if (NetworkInterface.connectedWifi) {
                    // Check contractor info once without retrying in the initial setup
                    deviceController.deviceControllerCPP.checkContractorInfo();
                }

                //! If privacy policy not accepted in normal mode load the PrivacyPolicyPage
                if (appModel.userPolicyTerms.acceptedVersion === appModel.userPolicyTerms.currentVersion) {
                    if ((openFromNoWiFiInstallation && NetworkInterface.connectedWifi)) {
                        root.StackView.view.push("qrc:/Stherm/View/ServiceTitan/CustomerDetailsPage.qml", {
                                                     "uiSession": uiSession,
                                                     "initialSetup": root.initialSetup,
                                                     "openFromNoWiFiInstallation": root.openFromNoWiFiInstallation
                                                 });

                    }

                    if (!openFromNoWiFiInstallation) {
                        root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                                     "uiSession": uiSession,
                                                     "initialSetup": root.initialSetup
                                                 });
                    }

                } else {
                    root.StackView.view.push("qrc:/Stherm/View/PrivacyPolicyPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "initialSetup": root.initialSetup,
                                                 "openFromNoWiFiInstallation": root.openFromNoWiFiInstallation
                                             });
                }
            }
        }
    }

    function isSecuredByWPA3(security: string)
    {
        security = security.toUpperCase();

        // SAE is WPA3 in the IW command
        const isWPA3Secured = !security.includes("PSK") &&
                            security.includes("SAE");

        if (isWPA3Secured) {
            console.log("securedByWPA3, security: ", security)
        }

        return isWPA3Secured
    }
}
