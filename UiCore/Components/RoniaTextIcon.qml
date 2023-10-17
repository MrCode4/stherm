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
    font.family: "Font Awesome 6 Pro"
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
    font.weight: 900
    color: enabled ? Material.foreground : Material.hintTextColor
}
