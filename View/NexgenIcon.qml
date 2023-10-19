import QtQuick

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Icon of NEXGEN
 * \todo This should be designed as svg and displayed using Image
 * ***********************************************************************************************/
Label {
    id: _root

    /* Object properties
     * ****************************************************************************************/
    font {
        family: "monospace"
        pointSize: AppStyle.size / 7.5
        capitalization: "AllUppercase"
    }
    textFormat: "RichText"
    text: `<b style="color:#1758AB;">nex</b><b style="color:#FD0029;">gen</b>`
}
