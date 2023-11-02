import QtQuick
import QtQuick.Layouts

import Ronia
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
            if (connectedIndex > -1) {
                wifis.unshift(wifis.splice(connectedIndex, 1)[0]);
            }
        }
        return wifis.sort((a, b) => b.strength - a.strength).filter((element, index) => element.ssid !== "");
    }

    /* Object properties
     * ****************************************************************************************/
    title: "Wifi Settings"
    topPadding: bottomPadding + 12

    /* Children
     * ****************************************************************************************/
    RowLayout {
        parent: _root.header.contentItem

        Switch {
            id: _wifiOnOffSw

            enabled: !NetworkInterface.isRunning
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

    //! Available networks Label
    Label {
        id: _availLbl
        parent: _wifisRepeater.count > 0 ? (sortedWifis[0]?.connected ? _wifisRepeater.itemAt(1)
                                                                      : _wifisRepeater.itemAt(0))
                                         : _root
        anchors {
            bottom: parent?.top ?? undefined
            left: parent?.left ?? undefined
        }
        visible: (parent instanceof WifiDelegate) && (sortedWifis[0].connected ? sortedWifis.length > 1
                                                                               : sortedWifis.length > 0)
        opacity: 0.7
        font.pointSize: Application.font.pointSize * 0.8
        text: "Available Network"
    }

    ColumnLayout {
        anchors.fill: parent

        //! Wifis ListView
        Flickable {
            id: _wifisFlick
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            contentWidth: width
            contentHeight: _wifisCol.implicitHeight

            //! Highlight Rectangle
            Rectangle {
                id: _hightlightRect
                readonly property alias wifiDelegate: _wifisRepeater.currentItem

                visible: wifiDelegate
                y: _availLbl.height + 8 //! Only init value, will be broken
                width: wifiDelegate?.width ?? 0
                height: wifiDelegate?.height ?? 0
                color: Style.listHighlightColor

                onWifiDelegateChanged: {
                    updatePos();
                }

                Behavior on y {
                    enabled: visible
                    NumberAnimation { duration: 150 }
                }

                Connections {
                    target: _hightlightRect.wifiDelegate

                    function onYChanged()
                    {
                        _hightlightRect.updatePos();
                    }
                }

                function updatePos()
                {
                    if (wifiDelegate) {
                        var newPos = wifiDelegate.mapToItem(_wifisFlick, 0, 0);
                        x = newPos.x;
                        y = newPos.y;
                    }
                }
            }

            ColumnLayout {
                id: _wifisCol
                width: parent.width
                spacing: 8

                Label {
                    visible: sortedWifis[0]?.connected ?? false
                    opacity: 0.7
                    font.pointSize: Application.font.pointSize * 0.8
                    text: "Current Network"
                }

                Repeater {
                    id: _wifisRepeater

                    property int currentIndex: -1
                    readonly property WifiDelegate currentItem: currentIndex > -1 && currentIndex < count
                                                                ? itemAt(currentIndex)
                                                                : null

                    model: sortedWifis
                    delegate: WifiDelegate {
                        id: _wifiDelegate

                        required property var modelData
                        required property int index

                        Layout.fillWidth: true
                        Layout.topMargin: (_availLbl.parent === this
                                           ? _availLbl.implicitHeight
                                             + (sortedWifis[0]?.connected ? 16 : 0)
                                           : 0)

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

                    } else {
                        //! Forget connection
                        if (_wifisRepeater.currentItem?.wifi) {
                            NetworkInterface.forgetWifi(_wifisRepeater.currentItem.wifi);
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            //! Connect/Disconnect button
            ButtonInverted {
                visible: _wifisRepeater.currentItem?.wifi ?? false
                text: _wifisRepeater.currentItem?.wifi.connected ? "Disconnect" : "Connect"

                onClicked: {
                    if (text === "Connect") {
                        var wifi = _wifisRepeater.currentItem.wifi;
                        var minPasswordLength = (wifi.security === "--" || wifi.security === "" ? 0 : 8)
                        var isSaved = NetworkInterface.isWifiSaved(wifi);

                        //! Open connect page
                        if (_root.StackView.view) {
                            //! Note: it's better to stop wifi refreshing to prevent any deleted
                            //! object access issues
                            _root.StackView.view.push("qrc:/Stherm/View/Wifi/WifiConnectPage.qml", {
                                                          "wifi": _wifisRepeater.currentItem.wifi,
                                                          "minPasswordLength": minPasswordLength,
                                                          "isSaved": isSaved,
                                                      })
                        }
                    } else {
                        //! Disconnect from this wifi
                        NetworkInterface.disconnectWifi(_wifisRepeater.currentItem.wifi);
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

    onSortedWifisChanged: _wifisRepeater.currentIndexChanged();
}
