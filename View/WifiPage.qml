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
        return wifis.sort((a, b) => b.strength - a.strength).filter((element, index) => element.ssid !== "");
    }

    property bool initialSetup: false

    /* Object properties
     * ****************************************************************************************/
    title: "Wi-Fi Settings"
    topPadding: bottomPadding + 12

    /* Children
     * ****************************************************************************************/

    //! Next button
    ToolButton {
        parent: root.header.contentItem

        visible: initialSetup

        contentItem: RoniaTextIcon {
            text: FAIcons.arrowRight
        }

        // Enable when the serial number is correctly filled
        enabled: initialSetup && deviceController.deviceControllerCPP.system.serialNumber.length > 0
        onClicked: {
            if (root.StackView.view) {
                root.StackView.view.push("qrc:/Stherm/View/SystemSetupPage.qml", {
                                             "uiSession": uiSession,
                                             "initialSetup": root.initialSetup
                                         });
            }
        }
    }

    RowLayout {
        parent: root.header.contentItem

        Switch {
            id: _wifiOnOffSw

            enabled: false && !NetworkInterface.isRunning
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
                visible: !NetworkInterface.isRunning
                contentItem: RoniaTextIcon {
                    text: "\uf2f9" //! rotate-right
                }

                onClicked: {
                    //! Force refresh
                    if (NetworkInterface.deviceIsOn && !NetworkInterface.isRunning) {
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
                running: NetworkInterface.isRunning
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
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8

            //! Manual button
            ButtonInverted {
                text: _wifisRepeater.currentItem?.wifi.connected ? "Forget" : "Manual"
                onClicked: {
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

            Item { Layout.fillWidth: true }

            //! Connect/Disconnect button
            ButtonInverted {
                visible: _wifisRepeater.currentItem?.wifi ?? false
                text: _wifisRepeater.currentItem?.wifi?.connected ? "Disconnect" : "Connect"

                onClicked: {
                    if (text === "Connect") {
                        var wifi = _wifisRepeater.currentItem.wifi;

                        //! Check if password for this wifi is saved.
                        if (NetworkInterface.isWifiSaved(wifi)) {
                            NetworkInterface.connectSavedWifi(wifi);
                        } else {
                            var minPasswordLength = (wifi.security === "--" || wifi.security === "" ? 0 : 8)
                            var isSaved = NetworkInterface.isWifiSaved(wifi);

                            //! Open connect page
                            if (root.StackView.view) {
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
                        NetworkInterface.disconnectWifi(_wifisRepeater.currentItem.wifi);
                    }
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
            //! Incorrect password entered
            if (root.StackView.view && root.StackView.view.currentItem === root) {
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

    onSortedWifisChanged: _wifisRepeater.currentIndexChanged();
}
