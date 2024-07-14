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

    property bool isPINWrong: false

    property bool showPin: false

    /* Object properties
     * ****************************************************************************************/

    implicitWidth: 40
    implicitHeight: 50
    echoMode: showPin ? TextInput.Normal : TextInput.Password

    validator: IntValidator {
        bottom: 0
        top: 9
    }

    leftPadding: 12
    topPadding: 2


    background: Rectangle {
        width: 40
        height: 50
        radius: 10
        color: Style.button.disabledColor

        border.width: 2
        border.color: isPINWrong ? Style.testFailColor :
                                   delegate.text.length > 0 ? Style.foreground : "transparent"
    }
}
