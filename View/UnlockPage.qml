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
     property Lock lock: appModel.lock

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

        Image {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            width: 37
            height: 49
            source: "qrc:/Stherm/Images/lock-image.svg"
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
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
                           var unLocked = deviceController.lock(false, pin);
                           updatePinStatus(unLocked);

                           clearPIN();
                       }
        }
    }
}
