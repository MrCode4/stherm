import QtQuick 2.0
import QtQuick.Controls

import QTemplateProject


/*! ***********************************************************************************************
 * This is the default RoniaButton. It holds a simple text object as contentItem.
 * ************************************************************************************************/
RoniaBaseButton {
    id: button

    /* Object Properties
     * ****************************************************************************************/
    font.bold: false
    font.pixelSize: height * 0.6

    /* Children
     * ****************************************************************************************/
    contentItem: Text {
        id: icon

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        font.family: "Arial"
        font.bold: button.font.bold
        font.pixelSize: button.font.pixelSize
        font.weight: 900

        color: !button.enabled ? "gray" : textColor


        text: button.text
    }
}
