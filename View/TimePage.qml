import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * TimePage provides a user interface for setting time
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Setting
    property Setting    setting: uiSession?.appModel?.setting ?? null

    /* Object properties
     * ****************************************************************************************/
    leftPadding: 16 * scaleFactor
    rightPadding: 16 * scaleFactor
    title: "Time"

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay

        anchors.fill: parent
        spacing: 16

        Label {
            font.pointSize: root.font.pointSize * 0.8
            text: "Time"
        }

        RowLayout {
            Label {
                Layout.fillWidth: true
                text: "Set time automatically"
            }

            Switch {
                id: autoTimeSwh
                checked: true
            }
        }

        RowLayout {
            id: timeLay
            Layout.topMargin: 4
            Layout.rightMargin: autoTimeSwh.rightPadding * 2
            enabled: !autoTimeSwh.checked

            Label {
                Layout.fillWidth: true
                text: "Time"
            }

            Label {
                readonly property bool is12Hour: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12

                font.letterSpacing: 1.5
                text: (new Date).toLocaleTimeString(Qt.locale(), "hh:mm" + (is12Hour ? " AP" : ""))
            }
        }

        //! Daylight Saving Time
        RowLayout {
            Layout.topMargin: 24

            Label {
                Layout.fillWidth: true
                text: "Daylight Saving Time"
            }

            Switch {
                id: dstSwh
                checked: false
            }
        }

        Item { Layout.fillHeight: true }
    }
}
