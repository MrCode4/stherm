import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as Template

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ContractorInformationFinishPage
 * ***********************************************************************************************/
BasePageView {
    id: root

    title: "Finish Contractor Flow"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        width: parent.width * 0.65

        ButtonInverted {
            Layout.fillWidth: true
            text: "   Forget Wi-Fis   "

            onClicked: {
                forgetWiFiPopup.cancelEnable = true;
                forgetWiFiPopup.open()
            }
        }

        ButtonInverted {
            Layout.fillWidth: true
            text: "   Restart Device   "

            onClicked: {
                rebootPopup.cancelEnable = true;
                rebootPopup.open();
            }
        }
    }

    //! Reboot popup with count down timer to send reboot request to system
    CountDownPopup {
        id: forgetWiFiPopup

        anchors.centerIn: Template.Overlay.overlay

        title: "Forget Wi-Fis"
        actionText: "Forgetting Wi-Fis..."

        onOpened: {
            // this should be reset on each show!
            cancelEnable = true;
        }

        onStartAction: {
            cancelEnable = false;
            NetworkInterface.forgetAllWifis();
        }
    }

    //! Reboot popup with count down timer to send reboot request to system
    CountDownPopup {
        id: rebootPopup

        anchors.centerIn: Template.Overlay.overlay

        title: "   Restart Device   "
        actionText: "Restarting Device..."

        onStartAction: {
            if (deviceController.system) {
                deviceController.system.rebootDevice();
            }
        }
    }

    property Connections networkInterface: Connections {
        target: NetworkInterface

        function onAllWiFiNetworksForgotten() {
            forgetWiFiPopup.close();
        }
    }
}
