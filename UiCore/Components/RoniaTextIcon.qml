import QtQuick
import QtQuick.Layouts

import Ronia

/*! ***********************************************************************************************
 * A simple text component for showing FontAwesome icons
 *
 * ************************************************************************************************/
Text {
    /* Object Properties
     * ****************************************************************************************/
    font.pointSize: Style.fontIconSize.normalPt
    font.family: "Font Awesome 6 Pro"
    font.weight: 900
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
    color: enabled ? Style.foreground : Style.hintTextColor
}
