import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UserGuidePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    title: "Technician Access"

    /* Children
     * ****************************************************************************************/
    Column {
        anchors.centerIn: parent
        width: parent.width * 0.55
        spacing: 4

        //! User guide link QR code
        Image {
            x: (parent.width - width) / 2
            width: parent.width
            height: width
            source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg("https://google.com", Style.foreground)}`
            sourceSize.width: width
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
        }

        Label {
            x: (parent.width - width) / 2
            text: "Please Follow the link"
        }
    }
}
