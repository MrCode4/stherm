import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm
import "./Delegates"
/*! ***********************************************************************************************
 * WifiPage provides a ui to connect to a Wifi network
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Wifi Settings"

    /* Children
     * ****************************************************************************************/
    //! Referesh button and running BusyIndicator
    Item {
        parent: _root.header
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

    //! Wifis ListView
    ListView {
        anchors.fill: parent
        model: NetworkInterface.wifis
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
                    _wifiConnectLay.visible = true
                    _wifiConnectLay.parent = _wifiDelegate;
                    _wifiConnectLay.y = _root.Material.delegateHeight + 8
                }
            }

            Connections {
                target: _wifiDelegate.wifi

                function onConnectedChanged() {
                    if (_wifiConnectLay.parent === _wifiDelegate) {
                        _wifiDelegate.clicked();
                    }
                }
            }
        }
    }

    //! Column for wifi password textfield and connect button
    ColumnLayout {
        id: _wifiConnectLay

        property bool isConnected: false

        width: parent ? parent.width - 32 : 0
        visible: false
        x: 16

        onParentChanged: _passwordTf.clear()

        TextField {
            id: _passwordTf
            Layout.fillWidth: true
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
            Layout.alignment: Qt.AlignRight
            leftPadding: 16
            rightPadding: 16
            enabled: _passwordTf.length > 0 && !NetworkInterface.isRunning
            text: "Connect"

            onClicked: {
                //! Connect to this wifi
                if (_wifiConnectLay.parent instanceof WifiDelegate) {
                    NetworkInterface.connectWifi(_wifiConnectLay.parent.wifi, _passwordTf.text)
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

    Component.onCompleted: {
        NetworkInterface.refereshWifis();
    }
}
