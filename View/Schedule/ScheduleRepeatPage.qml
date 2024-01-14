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
    property ScheduleCPP    schedule

    //! Repeats are always valid
    readonly property bool  isValid: repeats.length > 0

    property bool editMode: false

    //! Selected days for repeating
    readonly property string   repeats: {
        var rps = [];

        if (_moBtn.checked) rps.push("Mo");
        if (_tuBtn.checked) rps.push("Tu");
        if (_weBtn.checked) rps.push("We");
        if (_thBtn.checked) rps.push("Th");
        if (_frBtn.checked) rps.push("Fr");
        if (_saBtn.checked) rps.push("Sa");
        if (_suBtn.checked) rps.push("Su");

        return rps.join(",");
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
    titleHeadeingLevel: 4

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing and schedule (schedule is not null)
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
        visible: editMode
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! First check if this schedule has overlap with other Schedules
            //! Do this only if schedule is enabled (active)
            if (schedule.enable) {
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

        Label { Layout.alignment: Qt.AlignCenter; text: "Mo" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Tu" }
        Label { Layout.alignment: Qt.AlignCenter; text: "We" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Th" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Fr" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Sa" }
        Label { Layout.alignment: Qt.AlignCenter; text: "Su" }

        RadioButton {id: _moBtn; autoExclusive: false;}
        RadioButton {id: _tuBtn; autoExclusive: false }
        RadioButton {id: _weBtn; autoExclusive: false }
        RadioButton {id: _thBtn; autoExclusive: false }
        RadioButton {id: _frBtn; autoExclusive: false }
        RadioButton {id: _saBtn; autoExclusive: false }
        RadioButton {id: _suBtn; autoExclusive: false }
    }

    onScheduleChanged: {
        if (schedule) {
            _moBtn.checked = Boolean(schedule.repeats.includes("Mo"));
            _tuBtn.checked = Boolean(schedule.repeats.includes("Tu"));
            _weBtn.checked = Boolean(schedule.repeats.includes("We"));
            _thBtn.checked = Boolean(schedule.repeats.includes("Th"));
            _frBtn.checked = Boolean(schedule.repeats.includes("Fr"));
            _saBtn.checked = Boolean(schedule.repeats.includes("Sa"));
            _suBtn.checked = Boolean(schedule.repeats.includes("Su"));
        }
    }

    /* Methods
     * ****************************************************************************************/
    function saveRepeat()
    {
        //! If there is overlapping Schedules disable them
        internal.overlappingSchedules.forEach((element, index) => {
                                                  element.enable = false;
                                              });

        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(saveRepeat);

        if (schedule && schedule.repeats !== repeats) {
            schedule.repeats = repeats;
        }

        backButtonCallback();
    }
}
