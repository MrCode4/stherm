import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * WifiPage provides a ui to connect to a Wifi network
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    readonly property var sortedWifis: {
        var wifis = Array.from(NetworkInterface.wifis);

        if (wifis.length > 0) {
            var connectedIndex = wifis.findIndex((element) => {
                                                     return element.connected === true;
                                                 });
            wifis.unshift(wifis.splice(connectedIndex, 1)[0]);
        }
        return wifis;
    }

    /* Object properties
     * ****************************************************************************************/
    title: "Wifi Settings"
    topPadding: bottomPadding + 12

    /* Children
     * ****************************************************************************************/
    RowLayout {
        parent: _root.header

        Switch {
            id: _wifiOnOffSw

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
            implicitWidth: _refereshBtn.implicitWidth
            implicitHeight: _refereshBtn.implicitHeight

            ToolButton {
                id: _refereshBtn
                visible: !NetworkInterface.isRunning
                contentItem: RoniaTextIcon {
                    text: "\uf2f9" //! rotate-right
                }

                onClicked: {
                    //! Force refresh
                    NetworkInterface.refereshWifis(true);
                }
            }

            //! BusyIndicator for NetworkInterface running status
            BusyIndicator {
                anchors {
                    fill: parent
                    margins: 4
                }
                running: visible
                visible: NetworkInterface.isRunning
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Label {
            visible: sortedWifis.length > 0 && (sortedWifis[0]?.connected ?? false)
            font.pointSize: _root.font.pointSize * 0.85
            opacity: 0.7
            leftPadding: 8
            text: "Current Network"
        }

        //! WifiDelegate for currently connected Wifi
        WifiDelegate {
            id: _currentWifi
            Layout.fillWidth: true
            Layout.preferredHeight: + _root.Material.delegateHeight
                                    + (_wifiConnectLay.parent === this ? _wifiConnectLay.implicitHeight + 16 : 0)

            clip: true
            visible: sortedWifis.length > 0 && (sortedWifis[0]?.connected ?? false)
            wifi: sortedWifis.length > 0 && sortedWifis[0] instanceof WifiInfo ? sortedWifis[0] : null

            onClicked: {
                if (NetworkInterface.isRunning) {
                    return;
                }

                if (_wifiConnectLay.parent === this) {
                    _wifiConnectLay.visible = false;
                    _wifiConnectLay.parent = _root
                    _wifiConnectLay.y = 0;
                } else {
                    _wifiConnectLay.isConnected = true;
                    _wifiConnectLay.visible = true
                    _wifiConnectLay.parent = this;
                    _wifiConnectLay.y = _root.Material.delegateHeight + 8
                }
            }

            Behavior on Layout.preferredHeight { NumberAnimation { } }
        }

        Label {
            Layout.topMargin: _currentWifi.visible ? 20 : 0
            visible: sortedWifis.length > 0
            font.pointSize: _root.font.pointSize * 0.85
            opacity: 0.7
            leftPadding: 8
            text: "Available Networks"
        }

        //! Wifis ListView
        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: sortedWifis.length > 0
            clip: true
            model: sortedWifis.filter((element) => !element.connected)
            delegate: WifiDelegate {
                id: _wifiDelegate

                required property var modelData
                required property int index

                width: ListView.view.width
                height: + _root.Material.delegateHeight
                        + (_wifiConnectLay.parent === this ? _wifiConnectLay.implicitHeight + 16 : 0)
                clip: true

                wifi: modelData
                delegateIndex: index
                onClicked: {
                    if (NetworkInterface.isRunning) {
                        return;
                    }

                    if (_wifiConnectLay.parent === _wifiDelegate) {
                        _wifiConnectLay.visible = false;
                        _wifiConnectLay.parent = _root
                        _wifiConnectLay.y = 0;
                    } else if (!modelData.connected){
                        _wifiConnectLay.isConnected = false;
                        _wifiConnectLay.visible = true
                        _wifiConnectLay.parent = _wifiDelegate;
                        _wifiConnectLay.y = _root.Material.delegateHeight + 8
                    }
                }

                Connections {
                    target: _wifiDelegate.wifi

                    function onConnectedChanged() {
                        if (_wifiConnectLay.parent === _wifiDelegate) {
                            //! Also re-evaluate sortedWifis since this is changed too.
                            NetworkInterface.wifisChanged();
                            _wifiDelegate.clicked();
                        }
                    }
                }
            }
        }
    }
    //! Column for wifi password textfield and connect button
    GridLayout {
        id: _wifiConnectLay

        property bool isConnected: false

        width: parent ? parent.width - 32 : 0
        visible: false
        x: 16

        columns: 2
        columnSpacing: 8
        rowSpacing: 4

        onParentChanged: _passwordTf.clear()

        TextField {
            id: _passwordTf
            Layout.fillWidth: true
            Layout.columnSpan: 2

            visible: !_wifiConnectLay.isConnected
            maximumLength: 256
            rightPadding: _passwordEchoBtn.width
            placeholderText: "Enter Wifi password"
            echoMode: _passwordEchoBtn.checked ? TextField.Normal : TextField.Password

            ToolButton {
                id: _passwordEchoBtn
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    verticalCenterOffset: -6
                }
                checkable: true
                contentItem: RoniaTextIcon {
                    text: _passwordEchoBtn.checked ? "\uf070" : "\uf06e" //! eye-slash and eye button
                }
            }
        }

        Button {
            id: _forgetBtn
            Layout.alignment: Qt.AlignLeft
            Layout.preferredWidth: _connectBtn.width
            leftPadding: 16
            rightPadding: 16
            text: "Forget"

            onClicked: {
                //! Forget selected network
                if (_wifiConnectLay.parent instanceof WifiDelegate) {
                    NetworkInterface.forgetWifi(_wifiConnectLay.parent.wifi);
                }
            }
        }

        Button {
            id: _connectBtn
            Layout.alignment: Qt.AlignRight
            leftPadding: 16
            rightPadding: 16
            enabled: _wifiConnectLay.isConnected || (_passwordTf.length > 0 && !NetworkInterface.isRunning)
            text: _wifiConnectLay.isConnected ? "Disconnect" : "Connect"

            onClicked: {
                if (text === "Disconnect") {
                    //! Disconnect
                    NetworkInterface.disconnectWifi(_wifiConnectLay.parent.wifi)
                } else if (text === "Connect") {
                    //! Connect to this wifi
                    if (_wifiConnectLay.parent instanceof WifiDelegate) {
                        NetworkInterface.connectWifi(_wifiConnectLay.parent.wifi, _passwordTf.text)
                    }
                }
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

        function onErrorOccured(error, ssid)
        {
            _errorMessageLbl.text = `${error} ${ssid ? "**" + ssid + "**" : ""}`;
            _errorDrawer.open();
        }
    }
}
