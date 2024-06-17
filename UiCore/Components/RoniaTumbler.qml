import QtQuick
import QtQuick.Controls

import Ronia
import Stherm

/*! ***********************************************************************************************
 * RoniaTumbler is a modifued tumbler.
 * ***********************************************************************************************/


Tumbler {
    id: control

    /* Object properties
     * ****************************************************************************************/

    delegate: Text {
        text: formatText(Tumbler.tumbler.count, modelData)
        color: control.Material.foreground
        font: control.font
        opacity: (1.0 - Math.abs(Tumbler.displacement) / (control.visibleItemCount / 2)) * (control.enabled ? 1 : 0.6)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        required property var modelData
        required property int index
    }

    /* Functions
     * ****************************************************************************************/

    function formatText(count, modelData) {
        var data = modelData;
        return data.toString().length < 2 ? "0" + data : data;
    }

}
