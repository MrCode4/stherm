import QtQuick

import Ronia
import Stherm

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

    //! Finish button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Finish test, add delay to set relays
            deviceController.deviceControllerCPP.finalizeTesting()
            uiSession.showHome();
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
