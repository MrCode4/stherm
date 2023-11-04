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
    //! Wifi
    property WifiInfo   wifi

    //! Is this wifi saved
    property bool       isSaved

    //! Minimus password length
    property int        minPasswordLength: 0

    /* Object properties
     * ****************************************************************************************/
    title: ""
    titleHeadeingLevel: 6

    /* Children
     * ****************************************************************************************/
    RowLayout {
        parent: _root.header.contentItem

        //! wifi name
        Label {
            Layout.fillWidth: true
            textFormat: "MarkdownText"
            text: `${"#".repeat(Math.max(1, Math.min(6, titleHeadeingLevel)))} Password for "${wifi.ssid}"`
            horizontalAlignment: "AlignHCenter"
        }

        //! Connect button
        ToolButton {
            Layout.alignment: Qt.AlignRight

            enabled: _passwordTf.acceptableInput && !NetworkInterface.isRunning
            contentItem: RoniaTextIcon {
                text: FAIcons.link
            }

            onClicked: {
                //! Perform connection
                _connectCheckCon.enabled = true;
                if (isSaved) {
                    NetworkInterface.connectSavedWifi(wifi);
                } else {
                    NetworkInterface.connectWifi(wifi, _passwordTf.text);
                }
            }
        }
    }

    //! Password TextField
    TextField {
        id: _passwordTf

        anchors.centerIn: parent
        width: parent.width * 0.65

        maximumLength: 256
        rightPadding: _passwordEchoBtn.width
        placeholderText: "Enter Wifi password"
        echoMode: _passwordEchoBtn.checked ? TextField.Normal : TextField.Password
        validator: RegularExpressionValidator {
            regularExpression: new RegExp(`.{${minPasswordLength},${_passwordTf.maximumLength}}`)
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
        id: _connectCheckCon
        target: wifi
        enabled: false

        function onConnectedChanged()
        {
            if (wifi.connected) {
                if (_root.StackView.view) {
                    _root.StackView.view.pop();
                }
            }

            _connectCheckCon.enabled = false;
        }
    }
}
