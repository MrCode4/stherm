import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * A simple text component for showing FontAwesome icons
 *
 * ************************************************************************************************/
Text {
    /* Object Properties
     * ****************************************************************************************/
    font.pointSize: Style.fontIconSize.normalPt
    font.family: "Font Awesome 6 Pro"
    font.weight: FAIcons.Solid
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
    color: enabled ? Style.foreground : Style.hintTextColor
}
