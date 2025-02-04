import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiConnectPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Wi-Fi
    property WifiInfo   wifi

    //! Minimus password length
    property int        minPasswordLength: 0

    /* Object properties
     * ****************************************************************************************/
    title: ""
    titleHeadeingLevel: 4

    /* Children
     * ****************************************************************************************/
    RowLayout {
        parent: _root.header.contentItem

        //! wifi name
        Label {
            Layout.fillWidth: true
            textFormat: "MarkdownText"
            text: `${"#".repeat(Math.max(1, Math.min(6, titleHeadeingLevel)))} Password for\n"${wifi.ssid}"`
            horizontalAlignment: "AlignHCenter"
        }

        //! Connect button
        Item {
            Layout.alignment: Qt.AlignRight
            implicitWidth: _connectBtn.implicitWidth
            implicitHeight: _connectBtn.implicitHeight

            ToolButton {
                id: _connectBtn
                anchors.fill: parent
                visible: !NetworkInterface.busy
                enabled: _passwordTf.acceptableInput && !NetworkInterface.busy
                contentItem: RoniaTextIcon {
                    text: FAIcons.link
                }

                onClicked: {
                    //! Perform connection
                    NetworkInterface.connectWifi(wifi, _passwordTf.text);
                }
            }

            BusyIndicator {
                anchors.fill: parent
                anchors.margins: 4
                visible: NetworkInterface.busy
                running: visible
            }
        }
    }

    //! Password TextField
    TextField {
        id: _passwordTf

        anchors.centerIn: parent
        width: parent.width * 0.8

        maximumLength: 256
        rightPadding: _passwordEchoBtn.width
        placeholderText: `Password For\n"${wifi.ssid}"`
        echoMode: _passwordEchoBtn.checked ? TextField.Normal : TextField.Password
        validator: RegularExpressionValidator {
            regularExpression: new RegExp(`.{${minPasswordLength},${_passwordTf.maximumLength}}`)
        }

        onAccepted: {
            //! Perform connection
            _connectBtn.clicked();
            _connectBtn.forceActiveFocus();
        }

        ToolButton {
            id: _passwordEchoBtn
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: -6
            }
            checkable: true
            focusPolicy: "NoFocus"
            contentItem: RoniaTextIcon {
                font.pointSize: Style.fontIconSize.smallPt
                text: _passwordEchoBtn.checked ? FAIcons.eyeSlash : FAIcons.eye
            }
        }
    }

    Connections {
        target: NetworkInterface
        enabled: visible

        function onConnectedWifiChanged() {
            if (wifi.connected) {
                if (_root.StackView.view) {
                    _root.StackView.view.pop();
                }
            }
        }
    }

    StackView.onActivated: {
        _passwordTf.forceActiveFocus();
    }

    Component.onCompleted: {
        //! Disable wifi refresh
        if (uiSession) {
            uiSession.refreshWifiEnabled = false;
        }
    }

    Component.onDestruction: {
        //! Enable wifi refresh
        if (uiSession) {
            uiSession.refreshWifiEnabled = true;
            NetworkInterface.refereshWifis();
        }
    }
}
