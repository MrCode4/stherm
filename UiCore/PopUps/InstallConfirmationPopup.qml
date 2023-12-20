import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * InstallConfirmationPopup start the update procedure and request user authorization to restart the application..
 * ***********************************************************************************************/

I_PopUp {
    /* Object properties
     * ****************************************************************************************/
    title: ""

    property DeviceController deviceController

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        width: parent?.width ?? 0
        anchors.centerIn: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 32

        RoniaTextIcon {
            id: icon

            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: FAIcons.circleCheck
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 0.75
            wrapMode: Text.WordWrap
            text: "Updates has been successefully downloaded.\nThe application update requires a restart. Would you like to proceed?"
            horizontalAlignment: Text.AlignLeft
        }


        RowLayout {
            Layout.fillWidth: true

            Item {
                Layout.fillWidth: true
            }

            ButtonInverted {
                id: okButton

                Layout.fillHeight: true
                leftPadding: 8
                rightPadding: 8
                text: "Ok"

                onClicked: {
                    deviceController.deviceControllerCPP.system.updateAndRestart();
                }
            }
        }

    }
}
