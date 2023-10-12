import QtQuick
import QtQuick.Controls

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
        pointSize: 64
        capitalization: "AllUppercase"
    }
    textFormat: "RichText"
    text: `<b style="color:#1758AB;">nex</b><b style="color:#FD0029;">gen</b>`
}
