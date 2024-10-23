import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UserGuidePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    objectName: "TechnicianAccess"

    /* Object Declaration
     * ****************************************************************************************/

    property System system: deviceController.deviceControllerCPP.system

    //! Use in unlock page
    property bool openFromUnlockPage: false

    /* Object properties
     * ****************************************************************************************/
    title: "Technician Access"

    /* Children
     * ****************************************************************************************/

    Text {
        anchors.centerIn: parent

        wrapMode: Text.WordWrap
        text: "Your device is still not detected,\nEnsure your internet connection is active or\ncontact the manufacturer."
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        color: enabled ? Style.foreground : Style.hintTextColor
        visible: system.serialNumber.length === 0
        font.pointSize: Application.font.pointSize * 0.8
    }

    Column {
        anchors.centerIn: parent
        width: parent.width * 0.55
        spacing: 4
        visible: system.serialNumber.length > 0

        //! User guide link QR code
        Image {
            id: qrCodeImage

            property string url: appModel.contactContractor.technicianURL

            x: (parent.width - width) / 2
            width: parent.width
            height: width
            source: `data:image/svg+xml;utf8,${QRCodeGenerator.getQRCodeSvg(qrCodeImage.url, Style.foreground)}`
            sourceSize.width: width
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
        }

        Label {
            horizontalAlignment: Qt.AlignHCenter
            x: (parent.width - width) / 2
            font.pointSize: Application.font.pointSize * 0.9
            text: "For any issues or questions, please contact\nour tech support by calling\n(657) 626-4887."
        }
    }
}
