import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * HoldPopup displays a Popup to select hold status
 * ***********************************************************************************************/
Popup {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! UiSession
    property UiSession  uiSession

    //! App model
    property I_Device   device: uiSession.appModel

    /* Object properties
     * ****************************************************************************************/
    horizontalPadding: 96
    topPadding: 16
    bottomPadding: 48
    modal: true
    dim: true

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay
        anchors.centerIn: parent
        spacing: 12

        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 32
            text: "Hold"
        }

        Button {
            id: offBtn
            horizontalPadding: 32
            autoExclusive: true
            checkable: true
            text: "Off"
            checked: device?.isHold

            onToggled: {
                if (device && device.isHold !== checked) {
                    device.isHold = checked;
                }
            }
        }

        Button {
            id: onBtn
            horizontalPadding: 32
            autoExclusive: true
            checkable: true
            text: "On"
            checked: device?.isHold === false

            onToggled: {
                if (device && device.isHold === checked) {
                    device.isHold = !checked;
                }
            }
        }
    }
}
