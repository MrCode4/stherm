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

    property bool initialSetup : false

    /* Object properties
     * ****************************************************************************************/
    title: "Technician Access"

    Component.onCompleted: {
        if (initialSetup) {
            deviceController.pushSettings();
        }
    }

    Component.onDestruction: {
        if (initialSetup) {
            deviceController.pushSettings();
        }
    }

    /* Children
     * ****************************************************************************************/

    //! Start a timer once they are in technician page and check hasClient (checkSN) every 30 seconds
    Timer {
        repeat: true
        running: root.visible && initialSetup
        interval: 30000

        onTriggered: {
            deviceController.deviceControllerCPP.checkSN();
        }
    }

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
            x: (parent.width - width) / 2
            text: "Please Follow the link"
        }
    }
}
