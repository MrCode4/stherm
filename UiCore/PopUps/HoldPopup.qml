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
    property UiSession          uiSession

    //! Device controller
    property I_DeviceController deviceController:   uiSession?.deviceController ?? null

    //! App model
    property I_Device   device: uiSession.appModel

    onOpened: deviceController.updateEditMode(AppSpec.EMHold);

    onClosed: deviceController.updateEditMode(AppSpec.EMNone);

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
            checked: !(device?._isHold ?? false)
        }

        Button {
            id: onBtn
            horizontalPadding: 32
            autoExclusive: true
            checkable: true
            text: "On"
            checked: device?._isHold ?? false

            //! Using onCheckedChanged instead on onToggled to cover offBtn being checked too and avoid redundancy
            onCheckedChanged: {
                if (device && deviceController && device._isHold !== checked) {
                    deviceController.updateHold(checked)
                    delayedCloseTmr.running = true;
                }
            }
        }
    }

    Timer {
        id: delayedCloseTmr
        interval: 250
        onTriggered: root.close();
    }
}
