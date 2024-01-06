import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Notify the user when new update is available
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property Declaration
     * ****************************************************************************************/


    /* Object properties
     * ****************************************************************************************/
    title: ""

    /* Signals
     * ****************************************************************************************/
    signal openUpdatePage();

    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        id: mainLay

        width: parent?.width ?? 0
        anchors.centerIn: parent
        Layout.topMargin: 10
        spacing: 32

        RoniaTextIcon {
            id: icon

            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: FAIcons.circleInfo
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pointSize:  Application.font.pointSize * 0.75
            text: "A new update is available.\nWould you like to proceed with the update now?"
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true

            ButtonInverted {
                id: ignoreButton

                Layout.fillHeight: true
                leftPadding: 8
                rightPadding: 8

                text: "Later"

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
                text: "Update Now"

                onClicked: {
                    openUpdatePage();

                    close();
                }
            }
        }
    }
}
