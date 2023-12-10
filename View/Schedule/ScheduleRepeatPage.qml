import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleRepeatePage provides ui for setting repeats in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! schedulesModel: use to create new Schedule instance
    property SchedulesController     schedulesController: uiSession?.schedulesController ?? null

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property Schedule       schedule

    //! Repeats are always valid
    readonly property bool  isValid: true

    //! Selected days for repeating
    readonly property var   repeats: {
        var rps = [];

        if (_muBtn.checked) rps.push("Mu");
        if (_tuBtn.checked) rps.push("Tu");
        if (_weBtn.checked) rps.push("We");
        if (_thBtn.checked) rps.push("Th");
        if (_frBtn.checked) rps.push("Fr");
        if (_saBtn.checked) rps.push("Sa");
        if (_suBtn.checked) rps.push("Su");

        return rps;
    }

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: Math.max(_contentLay.implicitWidth, implicitHeaderWidth) + leftPadding + rightPadding
    implicitHeight: _contentLay.implicitHeight + implicitHeaderHeight + implicitFooterHeight + topPadding + bottomPadding
    leftPadding: 8 * scaleFactor
    rightPadding: 8 * scaleFactor
    font.bold: true
    title: "Repeat"
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing and schedule (schedule is not null)
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
        visible: schedule
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! First check if this schedule has overlap with other Schedules
            //! Do this only if schedule is enabled (active)
            if (schedule.active) {
                internal.overlappingSchedules = schedulesController.findOverlappingSchedules(
                            Date.fromLocaleTimeString(Qt.locale(), schedule.startTime, "hh:mm AP"),
                            Date.fromLocaleTimeString(Qt.locale(), schedule.endTime, "hh:mm AP"),
                            repeats,
                            schedule);

                if (internal.overlappingSchedules.length > 0) {
                    //! New schedules overlapps with at least one other Schedule
                    uiSession.popUps.scheduleOverlapPopup.accepted.connect(saveRepeat);
                    uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);
                    return;
                }
            }

            saveRepeat();
        }
    }

    QtObject {
        id: internal

        property var overlappingSchedules: []
    }

    GridLayout {
        id: _contentLay
        width: parent.width
        anchors.centerIn: parent
        rowSpacing: 12
        columns: 7

        Label { Layout.alignment: Qt.AlignCenter; text: "Mu" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Tu" }
        Label { Layout.alignment: Qt.AlignCenter; text: "We" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Th" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Fr" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Sa" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Su" }

        RadioButton {id: _muBtn; autoExclusive: false; checked: true }
        RadioButton {id: _tuBtn; autoExclusive: false }
        RadioButton {id: _weBtn; autoExclusive: false }
        RadioButton {id: _thBtn; autoExclusive: false }
        RadioButton {id: _frBtn; autoExclusive: false }
        RadioButton {id: _saBtn; autoExclusive: false }
        RadioButton {id: _suBtn; autoExclusive: false }
    }

    onScheduleChanged: {
        if (schedule) {
            _muBtn.checked = Boolean(schedule.repeats.find(element => element === "Mu"));
            _tuBtn.checked = Boolean(schedule.repeats.find(element => element === "Tu"));
            _weBtn.checked = Boolean(schedule.repeats.find(element => element === "We"));
            _thBtn.checked = Boolean(schedule.repeats.find(element => element === "Th"));
            _frBtn.checked = Boolean(schedule.repeats.find(element => element === "Fr"));
            _saBtn.checked = Boolean(schedule.repeats.find(element => element === "Sa"));
            _suBtn.checked = Boolean(schedule.repeats.find(element => element === "Su"));
        }
    }

    /* Methods
     * ****************************************************************************************/
    function saveRepeat()
    {
        //! If there is overlapping Schedules disable them
        internal.overlappingSchedules.forEach((element, index) => {
                                                  element.active = false;
                                              });

        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(saveRepeat);

        if (schedule && schedule.repeats.toString() !== repeats.toString()) {
            schedule.repeats = repeats;
        }

        backButtonCallback();
    }
}
