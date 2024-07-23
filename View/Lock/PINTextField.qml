import QtQuick
import QtQuick.Controls

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A delegate to be used in Lock/Unlock page
 * ***********************************************************************************************/

TextField {
    id: delegate

    /* Property declaration
     * ****************************************************************************************/

    property bool isPinWrong: false

    property bool showPin: false

    /* Object properties
     * ****************************************************************************************/

    implicitWidth: 35
    implicitHeight: 45
    echoMode: showPin ? TextInput.Normal : TextInput.Password

    validator: IntValidator {
        bottom: 0
        top: 9
    }

    horizontalAlignment: TextInput.AlignHCenter
    verticalAlignment: TextInput.AlignVCenter

    background: Rectangle {
        width: 40
        height: 50
        radius: 10
        color: Style.button.disabledColor

        border.width: 2
        border.color: isPinWrong ? Style.testFailColor :
                                   delegate.text.length > 0 ? Style.foreground : "transparent"
    }
}
