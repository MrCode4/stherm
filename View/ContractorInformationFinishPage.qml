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

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        spacing: 10

        ButtonInverted {
            Layout.fillWidth: true
            text: "   Forget Wi-Fis   "

            onClicked: {
                // TODO
                forgetWiFiPopup.cancelEnable = true;
                forgetWiFiPopup.open()
            }
        }

        ButtonInverted {
            Layout.fillWidth: true
            text: "   Restart App   "

            onClicked: {
                rebootPopup.cancelEnable = true;
                rebootPopup.open();
            }
        }
    }

    //! Reboot popup with count down timer to send reboot request to system
    RebootDevicePopup {
        id: forgetWiFiPopup

        anchors.centerIn: Template.Overlay.overlay

        title: "Forget Wi-Fis"
        infoText: "Forgetting Wi-Fis ..."

        onOpened: {
            cancelEnable = true;
        }

        onStartAction: {
            cancelEnable = false;
            NetworkInterface.forgetAllWifis();
        }
    }

    //! Reboot popup with count down timer to send reboot request to system
    RebootDevicePopup {
        id: rebootPopup

        anchors.centerIn: Template.Overlay.overlay

        onStartAction: {
            if (deviceController.system) {
                deviceController.system.rebootDevice();
            }
        }
    }

    property Connections networkInterface: Connections {
        target: NetworkInterface

        function onForgettingAllWifisChanged() {
            if (!NetworkInterface.forgettingAllWifis) {
                forgetWiFiPopup.close();
            }
        }
    }
}
