import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UpdateInterruptionPopup show when the download process interrupted.
 * ***********************************************************************************************/

I_PopUp {

    /* Object properties
     * ****************************************************************************************/
    title: ""

    /* Property Declaration
     * ****************************************************************************************/

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
            text: FAIcons.triangleExclamation
        }

        Label {

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            color: Style.foreground
            text: "       Update Interrupted       "
        }


        RowLayout {
            Layout.fillWidth: true

            ButtonInverted {
                id: ignoreButton

                Layout.fillHeight: true
                leftPadding: 8
                rightPadding: 8

                text: "ignore"

                onClicked: {
                    close();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ButtonInverted {
                id: retryButton

                Layout.fillHeight: true
                leftPadding: 8
                rightPadding: 8
                text: "retry"

                onClicked: {
                    deviceController.deviceControllerCPP.system.partialUpdate();
                }
            }
        }
    }
}
