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
                checked: DateTimeManager.autoUpdateTime

                onToggled: {
                    if (checked !== DateTimeManager.autoUpdateTime) {
                        //! Disable switch
                        autoTimeSwh.enabled = false;
                        //! Set onfinished of DateTimeManager
                        DateTimeManager.onfinish = () => { autoTimeSwh.enabled = true; };

                        //! Ask for changing auto update time
                        DateTimeManager.autoUpdateTime = checked;
                    }
                }
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

            onClicked: {
                //! Open SelectTimePage
                if (root.StackView.view) {
                    root.StackView.view.push(selectTimeCompo);
                } else {
                    selectTimeCompo.createObject(root.contentItem, {
                                                     "anchors.fill": root.contentItem
                                                 })
                }
            }
        }

        ItemDelegate {
            Layout.fillWidth: true
            horizontalPadding: 2
            contentItem: RowLayout {
                Layout.topMargin: 4
                spacing: 24

                Label {
                    Layout.fillWidth: true
                    text: "Time zone"
                }

                Label {
                    readonly property bool is12Hour: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12

                    Layout.rightMargin: autoTimeSwh.rightPadding * 2
                    Layout.fillWidth: true
                    text: `${DateTimeManager.currentTimeZone.id} ${DateTimeManager.currentTimeZone.offset}`
                    horizontalAlignment: "AlignRight"
                    elide: Qt.ElideRight
                }
            }

            onClicked: {
                //! Open SelectTimeZonePage
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

    Component {
        id: selectTimeCompo

        SelectTimePage {
            onTimeSelected: function(time) {
                DateTimeManager.setTime(Date.fromLocaleTimeString(Qt.locale(), time, "hh:mm:ss"));
            }
        }
    }
}
