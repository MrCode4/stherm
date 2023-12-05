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
        spacing: 6

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

        ItemDelegate {
            Layout.fillWidth: true
            horizontalPadding: 2
            contentItem: RowLayout {
                enabled: !autoTimeSwh.checked

                Label {
                    Layout.fillWidth: true
                    text: "Time"
                }

                Label {
                    readonly property bool is12Hour: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12

                    Layout.rightMargin: autoTimeSwh.rightPadding * 2
                    font.letterSpacing: 1.5
                    text: (new Date).toLocaleTimeString(Qt.locale(), "hh:mm" + (is12Hour ? " AP" : ""))
                }
            }
        }

        RowLayout {
            Label {
                Layout.fillWidth: true
                text: "Set time zone automatically"
            }

            Switch {
                id: autoTimezoneSwh
                checked: true
            }
        }

        ItemDelegate {
            Layout.fillWidth: true
            horizontalPadding: 2
            contentItem: RowLayout {
                Layout.topMargin: 4
                enabled: !autoTimezoneSwh.checked

                Label {
                    Layout.fillWidth: true
                    text: "Time zone"
                }

                Label {
                    readonly property bool is12Hour: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12

                    Layout.rightMargin: autoTimezoneSwh.rightPadding * 2
                    font.letterSpacing: 1.5
                    text: (new Date).toLocaleTimeString(Qt.locale(), "hh:mm" + (is12Hour ? " AP" : ""))
                }
            }
        }

        //! Daylight Saving Time
        RowLayout {

            Label {
                Layout.fillWidth: true
                text: "Daylight Saving Time"
            }

            Switch {
                id: dstSwh
                checked: false
            }
        }

        //! Daylight Saving Time
        RowLayout {
            Label {
                Layout.fillWidth: true
                text: "Use 12 Hour Format"
            }

            Switch {
                id: hourFortmatSwh
                checked: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12
            }
        }

        Item { Layout.fillHeight: true }
    }
}
