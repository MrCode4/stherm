import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleStartTimePage provides ui for selecting start time in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! schedulesModel: use to create new Schedule instance
    property SchedulesController     schedulesController: uiSession?.schedulesController ?? null

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property Schedule           schedule

    //! Schedule time property: end-time or start-time: only considered when schedule is set
    property string             timeProperty:   "start-time"

    //! Start time for schedule if this is end-time
    property date               startTime:      Date

    //! Min minutes diff between start and end time
    readonly property int       minTimeDiff:    120

    //! This shows whether the inputs in this page are valid or not
    readonly property bool      isValid:        {
        var selectedTimeDate = Date.fromLocaleTimeString(Qt.locale(), _contentLay.selectedTime, "hh:mm AP");
        return startTime && timeProperty === "end-time"
                ? (selectedTimeDate - startTime) / 60000 >= minTimeDiff
                : true
    }

    //! Time in string format: 'hh:mm AM/PM'
    property string    selectedTime

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: _contentLay.implicitWidth + leftPadding + rightPadding
    implicitHeight: _contentLay.implicitHeight + implicitHeaderHeight + implicitFooterHeight + topPadding + bottomPadding
    topPadding: 24
    backButtonVisible: false
    titleHeadeingLevel: 4
    title: timeProperty === "start-time" ? "Start Time" : "End Time"

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing and schedule (schedule is not null)
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
        visible: schedule
        enabled: isValid
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! First check if this schedule has overlap with other Schedules
            //! Do this only if schedule is enabled (active)
            if (schedule.active) {
                internal.overlappingSchedules = schedulesController.findOverlappingSchedules(
                            Date.fromLocaleTimeString(Qt.locale(),
                                                      timeProperty === "start-time" ? selectedTime
                                                                                    : schedule.startTime,
                                                      "hh:mm AP"),
                            Date.fromLocaleTimeString(Qt.locale(),
                                                      timeProperty === "end-time"  ? selectedTime
                                                                                   : schedule.endTime,
                                                      "hh:mm AP"),
                            schedule.repeats,
                            schedule);

                if (internal.overlappingSchedules.length > 0) {
                    //! New schedules overlapps with at least one other Schedule
                    uiSession.popUps.scheduleOverlapPopup.accepted.connect(saveTime);
                    uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);
                    return;
                }
            }

            saveTime();
        }
    }

    Binding {
        target: _root
        delayed: true
        property: "selectedTime"
        value: _contentLay.selectedTime
    }

    QtObject {
        id: internal

        property var overlappingSchedules: []
    }

    GridLayout {
        id: _contentLay

        readonly property string    selectedTime:   {
            var h = `${_hourTumbler.currentItem.modelData}`;
            var m = `${_minuteTumbler.currentItem.modelData}`;

            if (h.length === 1) h = "0" + h;
            if (m.length === 1) m = "0" + m;

            return `${h}:${m} ${_amRBtn.checked ? "AM" : "PM"}`;
        }

        anchors.centerIn: parent

        columns: 3
        columnSpacing: 12
        rowSpacing: 32

        Tumbler {
            id: _hourTumbler
            currentIndex: 0
            model: Array.from({ length: 12 }, (elem, indx) => indx + 1)

            Rectangle {
                x: 12
                y: parent.contentItem.delegateHeight * 2
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }

            Rectangle {
                x: 12
                y: parent.contentItem.delegateHeight * 3
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }
        }

        Label {
            textFormat: "MarkdownText"
            text: "# :"
            Layout.fillWidth:  true
        }

        Tumbler {
            id: _minuteTumbler
            currentIndex: 0
            model: 60

            Rectangle {
                x: 12
                y: parent.contentItem.delegateHeight * 2
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }

            Rectangle {
                x: 12
                y: parent.contentItem.delegateHeight * 3
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }
        }

        //! AM and PM radio buttons
        RadioButton {
            id: _amRBtn
            Layout.alignment: Qt.AlignCenter
            text: "AM"
            checked: true
        }

        Item { Layout.fillWidth:  true }

        RadioButton {
            id: _pmRBtn
            Layout.alignment: Qt.AlignCenter
            text: "PM"
        }
    }

    onScheduleChanged: {
        if (schedule) {
            var time
            if (timeProperty === "start-time") {
                time = schedule.startTime;
            } else if (timeProperty === "end-time") {
                time = schedule.endTime;
            }

            if (time) {
                setTimeFromString(time);
            }
        }
    }

    /* Methods
     * ****************************************************************************************/
    function saveTime()
    {
        //! If there is overlapping Schedules disable them
        internal.overlappingSchedules.forEach((element, index) => {
                                                  element.active = false;
                                              });

        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(saveTime);


        if (schedule) {
            if (timeProperty === "start-time" && schedule.startTime !== selectedTime) {
                schedule.startTime = selectedTime;
            } else if (timeProperty === "end-time" && schedule.endTime !== selectedTime) {
                schedule.endTime = selectedTime;
            }
        }

        backButtonCallback();
    }

    //!
    function setTimeFromString(time)
    {
        _hourTumbler.currentIndex = Number(time.slice(0, 2)) - 1;
        _minuteTumbler.currentIndex = Number(time.slice(3, 5));
        if (time.slice(6, 8) === "AM") {
            _amRBtn.checked = true;
        } else if (time.slice(6, 8) === "PM") {
            _pmRBtn.checked = true;
        }
    }
}
