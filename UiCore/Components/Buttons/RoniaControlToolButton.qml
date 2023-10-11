import QtQuick
import QtQuick.Controls
import QTemplateProject

/*! ***********************************************************************************************
 * Tool bar buttons
 * ************************************************************************************************/
ToolButton {
    id: toolButton

    /* Object Properties
     * ****************************************************************************************/
    width: 30
    height: 30
    text: "\uf2ed"
    font.family: "Font Awesome 6 Pro"
    font.pixelSize: 17

    /* Children
     * ****************************************************************************************/
    contentItem: Text {
        text: toolButton.text
        font: toolButton.font
        opacity: enabled ? 1.0 : 0.3
        color: "#75FFFFFF"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        id:toolButtonController
        width: toolButton.width
        height: toolButton.height
        radius: 5
        color: toolButton.hovered ? "#2f2f2f" : "transparent"
        opacity: enabled ? 1 : 0.3
        Behavior on color{ColorAnimation{duration: 75}}
    }
}
