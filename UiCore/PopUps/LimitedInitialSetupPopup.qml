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
    title: remainigTime > 0 ? "Limited Mode Active" : "Connection Required"
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

                if (remainigTime <= 0) {
                    rts = "00 hrs : 00 mins";
                    return rts;
                }

                var hours = Math.floor(remainigTime / 60000 / 60);
                var minutes = Math.floor((remainigTime - hours * 60000 * 60) / 60000);

                let hoursStr = hours.toString().padStart(2, '0');
                if (hours > 0) {
                    rts = `${hoursStr} hrs`;

                } else {
                    rts = "00 hrs"
                }

                let minutesStr = minutes.toString().padStart(2, '0');
                if (minutes > 0) {
                    rts += ` : ${minutesStr} mins`;

                } else {
                    rts += " : 00 mins";
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
            Layout.leftMargin: 15
            Layout.rightMargin: 15
            Layout.topMargin: 24
            spacing: 24

            ButtonInverted {
                Layout.fillWidth: true
                text: "   Connect to Wi-Fi   "

                onClicked: {
                    uiSession.openWifiPage(remainigTime > 0, true);
                    close();
                }
            }

            ButtonInverted {
                Layout.fillWidth: true
                text: "Close"
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
            visible: remainigTime <= 0 && false
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

