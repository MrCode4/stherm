import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QTemplateProject
import Qt.labs.platform

/*! ***********************************************************************************************
 * User Color Choice
 * ************************************************************************************************/
RoniaUserInputBase {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property alias  color:     colorDialog.color

    property alias  colorEditor: colorEditor

    /* Signals
     * ****************************************************************************************/
    signal edittingFinished()

    /* Object Properties
     * ****************************************************************************************/
    opacity: enabled ? 1 : 0.6

    editor: Rectangle {
        width: 26
        height: 26
        color: control.color
        radius: 5
        Rectangle {
            id: colorEditor
            width: parent.width
            height: parent.height
            radius: 5
            anchors.horizontalCenter: parent.horizontalCenter
            color: control.color
            MouseArea {
                anchors.fill: parent
                onClicked: colorDialog.open()
            }
        }

    }

    ColorDialog {
        id: colorDialog
        title: "Please Choose a Color"
        onAccepted: {
            control.edittingFinished();
        }
        onRejected: {
            colorDialog.close()
        }
    }

}
