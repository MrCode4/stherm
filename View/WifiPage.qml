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

    /* Object properties
     * ****************************************************************************************/
    title: "Wifi Settings"

    /* Children
     * ****************************************************************************************/
    //! Referesh button
    ToolButton {
        parent: _root.header
        contentItem: RoniaTextIcon {
            text: "\uf2f9" //! rotate-right
        }

        onClicked: {
            //! Force refresh
            NetworkInterface.refereshWifis(true);
        }
    }

    ListView {
        anchors.fill: parent
        model: NetworkInterface.wifis
        delegate: ItemDelegate {
            id: _wifiDelegate
            required property var modelData
            required property int index

            width: ListView.view.width
            height: + _root.Material.delegateHeight
                    + (_wifiConnectLay.parent === this ? _wifiConnectLay.implicitHeight + 16 : 0)
            clip: true

            RowLayout {
                x: 8
                width: parent.width - 16
                height: _root.Material.delegateHeight
                spacing: 12

                RoniaTextIcon {
                    color: modelData.connected ? _root.Material.accentColor : _root.Material.foreground
                    text: "\uf1eb" //! wifi icon
                }

                Label {
                    Layout.fillWidth: true
                    color: modelData.connected ? _root.Material.accentColor : _root.Material.foreground
                    text: modelData.ssid
                }
            }

            Behavior on height { NumberAnimation { } }

            onClicked: {
                if (_wifiConnectLay.parent === this) {
                    _wifiConnectLay.visible = false;
                    _wifiConnectLay.parent = _root
                    _wifiConnectLay.y = 0;
                } else if (!modelData.connected){
                    _wifiConnectLay.visible = true
                    _wifiConnectLay.parent = this;
                    _wifiConnectLay.y = _root.Material.delegateHeight + 8
                }
            }

            Connections {
                target: _wifiDelegate.modelData

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
            rightPadding: _passwordEchoBtn.width + 4
            placeholderText: "Enter Wifi password"
            echoMode: _passwordEchoBtn.checked ? TextField.Normal : TextField.Password

            ToolButton {
                id: _passwordEchoBtn
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
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
            enabled: _passwordTf.length > 0
            text: "Connect"

            onClicked: {
                //! Connect to this wifi
                if (_wifiConnectLay.parent instanceof ItemDelegate) {
                    NetworkInterface.connectWifi(_wifiConnectLay.parent.modelData, _passwordTf.text)
                }
            }
        }
    }

    Component.onCompleted: {
        NetworkInterface.refereshWifis();
    }
}
