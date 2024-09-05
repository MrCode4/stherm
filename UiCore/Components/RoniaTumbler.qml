import QtQuick
import QtQuick.Controls

import Ronia
import Stherm

/*! ***********************************************************************************************
 * RoniaTumbler is a modifued tumbler.
 * ***********************************************************************************************/


Tumbler {
    id: control

    property real tumblerViewHeight: 200
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

    contentItem: TumblerView {
        implicitWidth: 60
        implicitHeight: control.tumblerViewHeight
        model: control.model
        delegate: control.delegate
        path: Path {
            startX: control.contentItem.width / 2
            startY: -control.contentItem.delegateHeight / 2
            PathLine {
                x: control.contentItem.width / 2
                y: (control.visibleItemCount + 1) * control.contentItem.delegateHeight - control.contentItem.delegateHeight / 2
            }
        }

        property real delegateHeight: control.availableHeight / control.visibleItemCount
    }

    /* Functions
     * ****************************************************************************************/

    function formatText(count, modelData) {
        var data = modelData;
        return data.toString().length < 2 ? "0" + data : data;
    }

}
