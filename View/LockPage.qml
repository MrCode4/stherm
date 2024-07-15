import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * LockPage: Lock the app
 * ***********************************************************************************************/

BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property Lock lock: appModel.lock

    /* Object properties
     * ****************************************************************************************/
    title: "Lock"

    //! Contents
    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            font.pointSize: root.font.pointSize * 0.8
            text: "Type a 4 digit PIN code to\n      lock the thermostat"
            elide: Text.ElideMiddle
        }

        PINItem {
            id: pinItem

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            onSendPIN: pin => {
                           var locked = lock(pin);
                           updatePinStatus(locked);
                       }
        }
    }

    /* Functions
     * ****************************************************************************************/

    // Todo: lock the app
    function lock(pin: string) : bool {
        console.log("pin: ", pin);
        lock.isLock = pin === lock.pin;
        return lock.isLock;
    }
}
