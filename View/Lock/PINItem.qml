import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A delegate to be used in Lock/Unlock page
 * ***********************************************************************************************/

Control {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property int pinLength: 4

    /* Object properties
     * ****************************************************************************************/


    /* Children
     * ****************************************************************************************/

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width
        spacing: 8

        RowLayout {
            spacing: 8
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                model: pinLength

                delegate: TextField {
                    id: delegate

                    echoMode: TextInput.Password
                    validator: IntValidator {
                        bottom: 0
                        top: 9
                    }

                    background: Rectangle {
                        width: 35
                        height: 45
                        radius: 5
                        color: Style.button.disabledColor

                        border.width: delegate.text.legth > 0 ? 2 : 0
                        border.color: Style.foreground
                    }
                }
            }

        }


    }
}
