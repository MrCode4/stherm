import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Reboot popup with count down timer to send reboot request to system
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property Declaration
     * ****************************************************************************************/

    property System system

    /* Object properties
     * ****************************************************************************************/
    titleBar: false

    closePolicy: Popup.NoAutoClose

    onVisibleChanged: {
        if (visible) {
            mainLay.counter = 5;
        }
    }
    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay

        width: parent?.width ?? 0
        anchors.centerIn: parent
        Layout.topMargin: 10
        spacing: 32

        //! Add count down
        property int counter: 0

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * 1.5
            text: "   Reset Device   "
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.5
        }

        RoniaTextIcon {
            id: icon

            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: FAIcons.restart
            visible: mainLay.counter < 1
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * (mainLay.counter > 0 ? 1.5 : 0.9)
            text: mainLay.counter > 0 ? mainLay.counter : "Exiting Device..."
            horizontalAlignment: Text.AlignHCenter
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        ButtonInverted {

            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            visible: mainLay.counter > 0
            text: "Cancel"

            onClicked: {
                close();
            }
        }

        Timer {
            running: root.visible && mainLay.counter > 0
            repeat: true

            interval: 1000
            onTriggered: {

                mainLay.counter--;
                if (system && mainLay.counter <= 0) {
                    system.exitDevice();
                }
            }
        }
    }
}
