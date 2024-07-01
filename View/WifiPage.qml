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

    property bool initialSetup: false

    property bool initialSetupReady : initialSetup && system.serialNumber.length > 0 && uiSession.settingsReady && checkedUpdate

    //! Check update for first time
    property bool checkedUpdate: false;

    /* Object properties
     * ****************************************************************************************/
    title: "Wi-Fi Settings"
    topPadding: bottomPadding + 12

    /* Children
     * ****************************************************************************************/

    Timer {
        id: fetchTimer

        repeat: true
        running: root.visible && initialSetup && system.serialNumber.length > 0 && !uiSession.settingsReady
        interval: 5000

        onTriggered: {
            uiSession.settingsReady = system.fetchSettings();
        }
    }

    //! Once the network connection is established, the System Types page should automatically open,
    Timer {
        id: nextPageTimer

        property bool once : false

        repeat: false
        running: !once && root.visible && initialSetupReady
        interval: 10000
        onTriggered: nextPage()
    }

    //! Next button
    ToolButton {
        parent: root.header.contentItem

        visible: initialSetup

        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        // Enable when the serial number is correctly filled
        enabled: initialSetupReady
        onClicked: nextPage()
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
            delegateIndex: -1
            onClicked: {
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
                    readonly property WifiDelegate currentItem: currentIndex > -2 && currentIndex < count
                                                                ? currentIndex === -1 ? _connectedWifiDelegate
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
                        delegateIndex: index
                        onClicked: {
                            _wifisRepeater.currentIndex = index;
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

            ToolButton {
                anchors.centerIn: parent

                checkable: false
                checked: false
                visible: initialSetup
                implicitWidth: 64
                implicitHeight: implicitWidth
                icon.width: 50
                icon.height: 50

                contentItem: RoniaTextIcon {
                    anchors.fill: parent
                    font.pointSize: Style.fontIconSize.largePt
                    Layout.alignment: Qt.AlignLeft
                    text: FAIcons.circleInfo
                }

                onClicked: {
                    root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession)
                                             })

                }
            }

            //! Connect/Disconnect button
            ButtonInverted {
                anchors.right: parent.right
                anchors.rightMargin: 8
                visible: _wifisRepeater.currentItem?.wifi ?? false
                text: _wifisRepeater.currentItem?.wifi?.connected ? "Disconnect" : "Connect"

                onClicked: {
                    if (NetworkInterface.busy) {
                        return;
                    }

                    if (text === "Connect") {
                        var wifi = _wifisRepeater.currentItem.wifi;

                        //! Check if password for this wifi is saved.
                        if (NetworkInterface.isWifiSaved(wifi)) {
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
                    } else {
                        //! Disconnect from this wifi
                        if (_wifisRepeater.currentItem)
                            NetworkInterface.disconnectWifi(_wifisRepeater.currentItem.wifi);
                    }

                    _wifisRepeater.currentIndex = -2;
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

    Connections {
        target: NetworkInterface

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

    //! Change checkedUpdate when update checked in the initial setup
    Connections {
        target: system

        enabled: initialSetup && !checkedUpdate

        //! Check update
        function onUpdateNoChecked() {
            checkedUpdate = true;
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

                //! If privacy policy not accepted in normal mode load the PrivacyPolicyPage
                if (appModel.userPolicyTerms.acceptedVersion === appModel.userPolicyTerms.currentVersion) {
                    root.StackView.view.push("qrc:/Stherm/View/SystemSetup/SystemTypePage.qml", {
                                                 "uiSession": uiSession,
                                                 "initialSetup": root.initialSetup
                                             });

                } else {
                    root.StackView.view.push("qrc:/Stherm/View/PrivacyPolicyPage.qml", {
                                                 "uiSession": Qt.binding(() => uiSession),
                                                 "initialSetup": root.initialSetup
                                             });
                }
            }
        }
    }
}
