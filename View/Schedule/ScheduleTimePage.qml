import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleStartTimePage provides ui for selecting start time in AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! schedulesModel: use to create new Schedule instance
    property SchedulesController     schedulesController: uiSession?.schedulesController ?? null

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property ScheduleCPP        schedule

    //! Min minutes diff between start and end time
    readonly property int       minTimeDiff:    120

    property bool editMode:                 false

    //! This shows whether the inputs in this page are valid or not
    readonly property bool      isValid:        {
        var selectedEndTimeDate = Date.fromLocaleTimeString(Qt.locale(), selectedEndTime, "hh:mm AP");
        var selectedStartTimeDate = Date.fromLocaleTimeString(Qt.locale(), selectedStartTime, "hh:mm AP");

        var diffTime   = (selectedEndTimeDate - selectedStartTimeDate);

        if (diffTime < 0) {
            selectedEndTimeDate.setDate(selectedEndTimeDate.getDate() + 1);
            diffTime = (selectedEndTimeDate - selectedStartTimeDate);
        }

        return (diffTime / 60000 >= minTimeDiff);
    }

    //! Time in string format: 'hh:mm AM/PM'
    property string    selectedStartTime
    property string    selectedEndTime

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size // _contentLay.implicitWidth + leftPadding + rightPadding
    implicitHeight: AppStyle.size

    topPadding: 24
    backButtonVisible: false
    title: "Time"

    Component.onCompleted: {
        if (!editMode && schedule.type === AppSpec.Custom) {
            var now = new Date();
            //! Set start time to current time
            startTimeTumbler.setTimeFromString(now.toLocaleTimeString(Qt.locale(), "hh:mm AP"));

            //! Set selected time to 2 hours after schedule's start time
            var endTime = now;
            endTime.setTime(now.getTime() + 2 * 1000 * 60 * 60);

            endTimeTumbler.setTimeFromString(endTime.toLocaleTimeString(locale, "hh:mm AP"));
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing
    ToolButton {
        parent: schedule ? root.header.contentItem : root
        visible: editMode
        enabled: isValid
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            //! we do not need to check if this schedule has overlap with other Schedules
            //! it will be done before saving it!
            saveTime();
        }
    }

    Binding {
        target: root
        delayed: true
        property: "selectedStartTime"
        value: startTimeTumbler.selectedTime
    }

    Binding {
        target: root
        delayed: true
        property: "selectedEndTime"
        value: endTimeTumbler.selectedTime
    }

    Label {
        id: startTimeLabel

        anchors.top: parent.top
        anchors.topMargin: -10
        anchors.left: parent.left

        z: 1
        text: "Start Time:"
        font.pointSize: root.font.pointSize * 0.85

    }

    HorizontalTimeTumbler {
        id: startTimeTumbler

        anchors.top: startTimeLabel.bottom
        anchors.topMargin: -10
        anchors.horizontalCenter: parent.horizontalCenter

        Layout.alignment: Qt.AlignHCenter
    }

    Rectangle {
        id: spacerRect

        anchors.top: startTimeTumbler.bottom
        anchors.topMargin: -2
        anchors.horizontalCenter: parent.horizontalCenter

        height: 2
        width: parent.width * 0.7
        Layout.alignment: Qt.AlignHCenter

        color: AppStyle.primaryOffWhite
    }

    Label {
        id: endTimeLabel

        anchors.top: spacerRect.bottom
        anchors.topMargin: 12

        z: 1
        text: "End Time:"
        font.pointSize: root.font.pointSize * 0.85
    }

    HorizontalTimeTumbler {
        id: endTimeTumbler

        anchors.top: endTimeLabel.bottom
        anchors.topMargin: -10
        anchors.horizontalCenter: parent.horizontalCenter

        Layout.alignment: Qt.AlignHCenter
    }

    Label {
        id: msgLabel

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: endTimeTumbler.bottom

        opacity: isValid ? 0. : 1.
        width: parent.width * 0.8
        font.pointSize: root.font.pointSize * 0.85
        font.weight: Font.DemiBold
        wrapMode: "Wrap"
        text: "The minimum scheduling time duration is 2 hours"
        horizontalAlignment: Qt.AlignHCenter

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    onScheduleChanged: {
        if (schedule) {
            startTimeTumbler.setTimeFromString(schedule.startTime);
            endTimeTumbler.setTimeFromString(schedule.endTime);
        }
    }

    /* Methods
     * ****************************************************************************************/
    function saveTime()
    {
        if (editMode && schedule) {
            if (schedule.startTime !== selectedStartTime) {
                schedule.startTime = selectedStartTime;
            }

            if (schedule.endTime !== selectedEndTime) {
                schedule.endTime = selectedEndTime;
            }
        }

        backButtonCallback();
    }

}
