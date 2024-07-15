import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UnlockPage: unLock the app
 * ***********************************************************************************************/

BasePageView {
    id: root
    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: ""
    backButtonVisible: false
    header: null

    //! Contents
    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 8 * scaleFactor
        spacing: 8
        RoniaTextIcon {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            font.pointSize: root.font.pointSize * 3
            text: FAIcons.lock
            color: root.headerColor
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: root.font.pointSize * 0.8
            text: "Type the Pin to Unlock"
            elide: Text.ElideMiddle
        }

        PINItem {
            id: pinItem

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            isLock: false

            onSendPIN: pin => {
                           unlock(pin);
                       }
        }
    }


    /* Functions
     * ****************************************************************************************/

    // Todo: Unlock the app
    function unlock(pin: string) {
        console.log("unlock pin: ", pin);
    }
}
