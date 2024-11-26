import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * LimitedInitialSetupPopup
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Properties
    * ****************************************************************************************/
    property var uiSession
    property int remainigTime: 0

    /* Object properties
     * ****************************************************************************************/
    title: "Limited Mode Active"
    closePolicy: remainigTime > 0 ? (Popup.CloseOnReleaseOutside | Popup.CloseOnEscape) :
                                    Popup.NoAutoClose
    closeButtonEnabled: false
    topPadding: 20

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent?.width ?? 0
        anchors.centerIn: parent
        spacing: 16

        Label {
            property string remainigTimeString: {
                var rts = "";

                var hours = Math.floor(remainigTime / 60000 / 60);
                var minutes = Math.floor((remainigTime - hours * 60000 * 60) / 60000);

                if (hours >= 0) {
                    rts = `${hours} hrs`;
                }

                if (minutes >= 0) {
                    rts += ` : ${minutes} mins`;
                }

                return rts;
            }

            Layout.fillWidth: true

            textFormat: Text.RichText
            font.pointSize:  Qt.application.font.pointSize * 0.8
            text: remainigTime > 0 ? `This thermostat is operating in limited mode.<br><br>Remaining time until Wi-Fi connection is required<br><span style="font-weight:bold;color:#42E8FF">${remainigTimeString}</span>`
                                   : `The 100-hour limited mode has ended. Connect to Wi-Fi to continue using the thermostat.<br><br><span style="font-weight:bold;color:#42E8FF">${remainigTimeString}</span>`
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
                text: "  Connect to WiFi  "

                onClicked: {
                    uiSession.openWifiPage(remainigTime > 0);
                    close();
                }
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "close"
                visible: remainigTime > 0

                onClicked: {
                    close();
                }
            }
        }

        //! TODO: Check
        RoniaTextIcon {
            Layout.margins: 10
            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
            font.pointSize: Style.fontIconSize.smallPt
            visible: remainigTime <= 0
            text: FAIcons.headSet

            TapHandler {
                onTapped: {
                   uiSession.openUnlockPage();
                    close();
                }
            }
        }
    }
}

