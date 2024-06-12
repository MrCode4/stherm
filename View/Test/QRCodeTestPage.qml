import QtQuick

import Ronia
import Stherm
import QtQuick.Templates as Template

/*! ***********************************************************************************************
 * QRCodeTestPage: Generate a QR code based on {"hv":"01","uid":"000000000000000"} string
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property Declaration
     * ****************************************************************************************/
    property System system: deviceController.deviceControllerCPP.system

    /* Object properties
     * ****************************************************************************************/

    title: "System  QR Code "

    /* Children
     * ****************************************************************************************/

    //! Reboot popup with count down timer to send reboot request to system
    RebootDevicePopup {
        id: rebootPopup
        system: root.system
        infoText: "Serial number is ready.\nRestarting Device..."
        anchors.centerIn: Template.Overlay.overlay
    }

    InfoPopup {
        id: infoPopup
        message: "QR Code"
        detailMessage: "Serial number is not valid.<br>Please try again."
        visible: false
    }

    //! Finish button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Finish test, add delay to set relays
            deviceController.deviceControllerCPP.testFinished();

            var sn = system.serialNumber;

            if (sn.length === 0) {
                //! Get SN with uid.
                var uid = deviceController.deviceControllerCPP.deviceAPI.uid;
                system.getSN(uid);
            }

            if (sn.length > 0) {
                rebootPopup.open();

            } else {
                infoPopup.open();
            }
        }
    }

    //! User guide link QR code
    Image {
        id: qrCodeImage

        anchors.centerIn: parent

        property string url: "{\"hv\":\"01\",\"uid\":\"" + system.systemUID +"\"}"

        width: parent.width * 0.75
        height: width
        source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg(qrCodeImage.url, Style.foreground)}`
        sourceSize.width: width
        sourceSize.height: height
        fillMode: Image.PreserveAspectFit
    }

    ToolButton {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        checkable: false
        checked: false
        visible: true
        implicitWidth: 64
        implicitHeight: implicitWidth
        icon.width: 50
        icon.height: 50

        contentItem: RoniaTextIcon {
            anchors.fill: parent
            font.pointSize: Style.fontIconSize.largePt
            text: FAIcons.circleInfo
        }

        onClicked: {
            root.StackView.view.push("qrc:/Stherm/View/AboutDevicePage.qml", {
                                         "uiSession": Qt.binding(() => uiSession)
                                     })

        }
    }
}
