import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * LimitedModeRemainigTimePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

     property int remainigTime: deviceController.limitedModeRemainigTime


    /* Object properties
     * ****************************************************************************************/
    leftPadding: 8 * scaleFactor
    rightPadding: 12 * scaleFactor
    titleHeadeingLevel: 5
    title: "No Wi-Fi Remaining Time"

    backButtonCallback: function() {
        //! Check if domain is modified
        if (remainigTime === (remainingTimeTF.text * 1000)) {
            tryGoBack()

        } else {
            //! This means that changes are occured that are not saved into model
            uiSession.popUps.exitConfirmPopup.accepted.connect(confirmtBtn.clicked);
            uiSession.popUps.exitConfirmPopup.rejected.connect(tryGoBack);
            uiSession.popupLayout.displayPopUp(uiSession.popUps.exitConfirmPopup);
        }
    }

    Component.onCompleted: {
        remainingTimeTF.text = remainigTime / 1000;
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        id: confirmtBtn

        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! Save
            deviceController.limitedModeRemainigTime = remainingTimeTF.text * 1000;
            deviceController.system.setLimitedModeRemainigTime(deviceController.limitedModeRemainigTime);
            tryGoBack()
        }
    }

    ColumnLayout {
        id: _contentLay

        width: parent.width
        spacing: 0

        Label {
            Layout.fillWidth: true

            property string remainigTimeString: {
                var rts = "";

                var rt = remainingTimeTF.text
                if (rt <= 0) {
                   rts = "00 hrs : 00 mins";
                    return rts;
                }

                var hours = Math.floor(rt / 60 / 60);
                var minutes = Math.floor((rt - hours * 60 * 60) / 60);

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

            text: `By skipping the Wi-Fi, the thermostat will operate in a limited mode. <br>You will have full functionality for <SPAN STYLE="font-weight:bold"> ${remainigTimeString} </SPAN>.<br>After that period, a network connection is required.`
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            font.pointSize:  Qt.application.font.pointSize * 0.8
        }

        Item {
            id: spacer

            Layout.fillWidth: true
            height: 10 * scaleFactor
        }

        Label {
            text: "Remaining time (seconds):"
        }

        TextField {
            id: remainingTimeTF

            Layout.fillWidth: true
            placeholderText: "secs"
            validator: IntValidator{
                bottom: 0
            }
            inputMethodHints: Qt.ImhPreferNumbers
        }
    }

    //! Reset setting Button
    ButtonInverted {
        id: resetButton
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 10
        text: "Reset"
        onClicked: {
            remainingTimeTF.text = remainigTime / 1000;
        }
    }
}
