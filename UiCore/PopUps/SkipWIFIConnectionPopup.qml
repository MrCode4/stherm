import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SkipWIFIConnectionPopup: Get confirm to skip wifi connection.
 * ***********************************************************************************************/
I_PopUp {
    /* Signals
    * ****************************************************************************************/
    signal connectToWiFi()
    signal skipWiFi()

    /* Object properties
     * ****************************************************************************************/
    title: "Skip Wi-Fi Connection?"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.fillWidth: true

            textFormat: Text.RichText
            font.pointSize:  Qt.application.font.pointSize * 0.8
            text: `By skipping the Wi-Fi, the thermostat will operate in a limited mode. <br>You will have full functionality for <SPAN STYLE="font-weight:bold">100 hours</SPAN>.<br>After that period, a network connection is required.`
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.topMargin: 24
            spacing: 24

            ButtonInverted {
                Layout.fillWidth: true
                text: "Connect to WiFi"

                onClicked: {
                    connectToWiFi();
                    close();
                }
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "Skip"

                onClicked: {
                    skipWiFi();
                    close();
                }
            }
        }
    }
}
