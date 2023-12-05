import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SetTimePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal timeSelected(string time)

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Set System Time"

    /* Children
     * ****************************************************************************************/
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            timeSelected(hourTumbler.hour + ":" + minuteTumbler.minute + ":00");

            if (root.StackView.view) {
                root.StackView.view.pop();
            }
        }
    }

    RowLayout {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -48
        spacing: 32

        Tumbler {
            id: hourTumbler

            property string hour: (currentIndex + 1 < 10 ? "0" : "") + (currentIndex + 1)

            model: Array.from({ length: 23 }, (elem, indx) => indx + 1)
        }

        Tumbler {
            id: minuteTumbler

            property string minute: (currentIndex + 1 < 10 ? "0" : "") + (currentIndex + 1)

            model: Array.from({ length: 59 }, (elem, indx) => indx + 1)
        }
    }
}
