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
    title: "Date & Time"

    Component.onCompleted: deviceController.editMode = AppSpec.EMDateTime;
    Component.onDestruction: deviceController.editMode = AppSpec.EMNone;

    /* Children
     * ****************************************************************************************/
    ColumnLayout {
        id: mainLay

        anchors.fill: parent
        spacing: 4

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
            enabled: !autoTimeSwh.checked
            rightPadding: 4
            leftPadding: 8
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    text: "Time"
                }

                Label {
                    readonly property bool is12Hour: setting?.timeFormat === AppSpec.TimeFormat.Hour12

                    Layout.rightMargin: autoTimeSwh.rightPadding
                    font.letterSpacing: 1.5
                    text: DateTimeManager.now.toLocaleTimeString(locale, is12Hour ? "hh:mm AP" : "hh:mm");
                }
            }

            onClicked: {
                //! Open SelectTimePage
                if (root.StackView.view) {
                    root.StackView.view.push(selectTimeCompo);
                }
            }
        }

        ItemDelegate {
            Layout.fillWidth: true
            enabled: !autoTimeSwh.checked
            rightPadding: 4
            leftPadding: 8
            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true
                    text: "Date"
                }

                Label {
                    Layout.rightMargin: autoTimeSwh.rightPadding
                    font.letterSpacing: 1.5
                    text: DateTimeManager.now.toLocaleString(locale, "ddd MMM d, yyyy");
                }
            }

            onClicked: {
                //! Open SelectTimePage
                if (root.StackView.view) {
                    root.StackView.view.push(selectDateCompo);
                }
            }
        }

        ItemDelegate {
            Layout.fillWidth: true
            rightPadding: 4
            leftPadding: 8
            contentItem: RowLayout {
                Layout.topMargin: 4
                spacing: 24

                Label {
                    Layout.fillWidth: true
                    text: "Time zone"
                }

                Label {
                    readonly property bool is12Hour: setting?.timeFormat === AppSpec.TimeFormat.Hour12

                    Layout.rightMargin: autoTimeSwh.rightPadding
                    Layout.fillWidth: true
                    text: `${DateTimeManager.currentTimeZone.id} ${DateTimeManager.currentTimeZone.offset}`
                    horizontalAlignment: "AlignRight"
                    elide: Qt.ElideRight
                }
            }

            onClicked: {
                //! Open SelectTimeZonePage
                if (root.StackView.view) {
                    root.StackView.view.push(selectTimezoneCompo);
                }
            }
        }

        //! Daylight Saving Time
        RowLayout {
            enabled: DateTimeManager.hasDST

            Label {
                Layout.fillWidth: true
                text: "Daylight Saving Time"
            }

            Switch {
                id: dstSwh
                checked: DateTimeManager.effectDst

                onToggled: {
                    if (DateTimeManager.effectDst !== checked) {
                        DateTimeManager.effectDst = checked;
                    }
                }
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
                enabled: appModel
                checked: setting?.timeFormat === AppSpec.TimeFormat.Hour12

                onToggled: {
                    var isNeedToSendToServer = false;

                    if (checked && setting.timeFormat !== AppSpec.TimeFormat.Hour12) {
                        setting.timeFormat = AppSpec.TimeFormat.Hour12;
                        isNeedToSendToServer = true;

                    } else if (!checked && setting.timeFormat !== AppSpec.TimeFormat.Hour24) {
                        setting.timeFormat = AppSpec.TimeFormat.Hour24;
                        isNeedToSendToServer = true;
                    }

                    if (isNeedToSendToServer) {
                        // Send changes to server
                        deviceController.finalizeSettings();
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    Component {
        id: selectTimeCompo

        SelectTimePage {
            uiSession: root.uiSession
            onTimeSelected: function(time) {
                DateTimeManager.setDateTime(time);
            }
        }
    }

    Component {
        id: selectDateCompo

        SelectDatePage {
            onDateSelected: function(date) {
                var selectedDate = new Date;
                selectedDate.setDate(date.getDate());
                selectedDate.setMonth(date.getMonth());
                selectedDate.setFullYear(date.getFullYear());

                DateTimeManager.setDateTime(selectedDate);
            }
        }
    }

    Component {
        id: selectTimezoneCompo

        SelectTimezonePage {
            onTimezoneSelected: function(timezone) {
                DateTimeManager.currentTimeZone = timezone;
            }
        }
    }

    Component.onCompleted: {
        DateTimeManager.checkAutoUpdateTime();
    }
}
