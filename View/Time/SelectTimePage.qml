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

            Layout.preferredWidth: 96
            model: Array.from({ length: 24 }, (elem, indx) => indx)
            font.pointSize: metrics.font.pointSize

            Component.onCompleted: contentItem.delegateHeight = metrics.height * 1.8
        }

        Tumbler {
            id: minuteTumbler

            property string minute: (currentIndex + 1 < 10 ? "0" : "") + (currentIndex + 1)

            Layout.preferredWidth: 96
            model: Array.from({ length: 60 }, (elem, indx) => indx)
            font.pointSize: metrics.font.pointSize

            Component.onCompleted: contentItem.delegateHeight = metrics.height * 1.8
        }
    }

    TextMetrics {
        id: metrics
        font.pointSize: root.font.pointSize * 1.4
        text: "99"
    }

    Component.onCompleted: {
        var now = DateTimeManager.now;
        hourTumbler.currentIndex = now.getHours();
        minuteTumbler.currentIndex = now.getMinutes();
    }
}
