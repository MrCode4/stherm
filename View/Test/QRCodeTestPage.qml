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
            var sn = system.serialNumber;

            if (sn.length === 0) {
                //! Get SN with uid.
                var uid = deviceController.deviceControllerCPP.deviceAPI.uid;
                system.getSN(uid);
            }

            if (sn.length > 0) {
                //! Finish test, add delay to set relays
                deviceController.deviceControllerCPP.testFinished();
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
}
