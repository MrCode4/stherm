import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * WifiManualConnectPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 16 * scaleFactor
    rightPadding: 24 * scaleFactor
    title: "WiFi Manual"

    /* Children
     * ****************************************************************************************/
    ToolButton {
        enabled: !NetworkInterface.isRunning
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Save this wifi
            NetworkInterface.addConnection(_conNameTf.text,
                                           _conNameTf.text,
                                           _ipAddressTf.text + (_ipSubnetTf.length > 0 ? "/" + _ipSubnetTf.text : ""),
                                           _ipGateWayTf.text,
                                           _ipDns1Tf.text + " " + _ipDns2Tf.text,
                                           _securityCmbox.currentValue,
                                           _securityCmbox.currentIndex > 0 ? _ipPasswordTf.text: "");
            _exitConn.enabled = true;
        }

        Connections {
            id: _exitConn
            target: NetworkInterface
            enabled: false

            function onIsRunningChanged()
            {
                if (!NetworkInterface.isRunning) {
                    backButtonCallback();
                }
            }
        }
    }

    Flickable {
        id: _contentFlick

        ScrollIndicator.vertical: ScrollIndicator {
            parent: _contentFlick.parent
            anchors {
                left: parent.right
                top: parent.top
                bottom: parent.bottom
                leftMargin: _root.rightPadding / 2
            }
            active: _contentFlick.height < _contentFlick.contentHeight
        }

        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: _contentLay.implicitHeight + 4
        boundsMovement: Flickable.StopAtBounds

        GridLayout {
            id: _contentLay
            width: parent.width
            columns: 2
            columnSpacing: 16 * scaleFactor
            rowSpacing: 8 * scaleFactor

            //! IP Address
            Label {
                text: "Name"
            }

            TextField {
                id: _conNameTf
                Layout.fillWidth: true
                placeholderText: "Name"
            }

            //! IP Address
            Label {
                text: "IP Address"
            }

            TextField {
                id: _ipAddressTf
                Layout.fillWidth: true
                placeholderText: "IP Address"
            }

            //! Subnet Mask
            Label {
                text: "Subnet Mask"
            }

            TextField {
                id: _ipSubnetTf
                Layout.fillWidth: true
                placeholderText: "Subnet Mask"
            }

            //! Gateway
            Label {
                text: "Gateway"
            }

            TextField {
                id: _ipGateWayTf
                Layout.fillWidth: true
                placeholderText: "Gateway"
            }

            //! DNS 1
            Label {
                text: "DNS 1"
            }

            TextField {
                id: _ipDns1Tf
                Layout.fillWidth: true
                placeholderText: "DNS 1"
            }

            //! DNS 2
            Label {
                text: "DNS 2"
            }

            TextField {
                id: _ipDns2Tf
                Layout.fillWidth: true
                placeholderText: "DNS 2"
            }

            //! Security
            Label {
                text: "Security"
            }

            ComboBox {
                id: _securityCmbox
                Layout.fillWidth: true
                Layout.preferredHeight: _ipDns2Tf.height

                model: [
                    { "title": "None",                  "key": ""},
                    { "title": "WEP",                   "key": "wep"},
                    { "title": "WPA/WPA2-Personal",     "key": "wpa-psk"},
                    { "title": "WPA3-Personal",         "key": "sae"},
                ]
                textRole: "title"
                valueRole: "key"
            }

            //! Password
            Label {
                visible: _securityCmbox.currentIndex > 0
                text: "Password"
            }

            TextField {
                id: _ipPasswordTf
                Layout.fillWidth: true

                visible: _securityCmbox.currentIndex > 0
                placeholderText: "Password"
            }
        }
    }
}
