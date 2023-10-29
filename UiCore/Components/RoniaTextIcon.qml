import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

/*! ***********************************************************************************************
 * A simple text component for showing FontAwesome icons
 *
 * ************************************************************************************************/
Text {
    /* Object Properties
     * ****************************************************************************************/
    font.pointSize: Qt.application.font.pointSize
    font.family: "Font Awesome 6 Pro"
    font.weight: 900
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
    color: enabled ? Material.foreground : Material.hintTextColor
}
