import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * UpdatePopup: Currently use in nrf update
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property Declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/

    titleBar: false

    closePolicy: Popup.NoAutoClose

    enter: Transition {}

    exit: Transition {}

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        width: parent?.width ?? 0
        anchors.centerIn: parent
        Layout.topMargin: 20
        spacing: 32

        RoniaTextIcon {
            id: icon

            Layout.topMargin: 10
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: FAIcons.update
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize
            text: "      Applying updates,         \n        please wait....      "
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.5
        }

    }
}
