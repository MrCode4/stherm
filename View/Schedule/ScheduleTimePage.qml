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
    property ScheduleCPP        schedule

    //! Schedule time property: end-time or start-time: only considered when schedule is set
    property string             timeProperty:   "start-time"

    //! Start time for schedule if this is end-time
    property date               startTime:      Date

    //! Min minutes diff between start and end time
    readonly property int       minTimeDiff:    120

    property bool editMode:                 false

    //! This shows whether the inputs in this page are valid or not
    readonly property bool      isValid:        {
        var selectedTimeDate = Date.fromLocaleTimeString(Qt.locale(), _contentLay.selectedTime, "hh:mm AP");
        if (startTime && timeProperty === "end-time") {
            var diffTime   = (selectedTimeDate - startTime);

            if (diffTime < 0) {
                selectedTimeDate.setDate(selectedTimeDate.getDate() + 1);
                diffTime = (selectedTimeDate - startTime);
            }

            return (diffTime / 60000 >= minTimeDiff);

        } else {
            return true;
        }
    }

    //! Time in string format: 'hh:mm AM/PM'
    property string    selectedTime

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size // _contentLay.implicitWidth + leftPadding + rightPadding
    implicitHeight: _contentLay.implicitHeight + (msgLabel.visible ? msgLabel.implicitHeight : 0)
                    + implicitHeaderHeight + implicitFooterHeight + topPadding + bottomPadding
    topPadding: 24
    backButtonVisible: false
    titleHeadeingLevel: 4
    title: timeProperty === "start-time" ? "Start Time" : "End Time"

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
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
        target: _root
        delayed: true
        property: "selectedTime"
        value: _contentLay.selectedTime
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
        width: parent.width * 0.6

        columns: 3
        columnSpacing: 12
        rowSpacing: 20

        Item {
            Layout.alignment: Qt.AlignCenter
            implicitHeight: _hourTumbler.implicitHeight
            implicitWidth: _hourTumbler.implicitWidth

            readonly property real delegateHeight: _hourTumbler.availableHeight / _hourTumbler.visibleItemCount


            RoniaTumbler {
                id: _hourTumbler
                anchors.fill: parent
                currentIndex: 0
                model: Array.from({ length: 12 }, (elem, indx) => indx + 1)

            }
            Rectangle {
                x: 12
                y: parent.delegateHeight * 2
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }

            Rectangle {
                x: 12
                y: parent.delegateHeight * 3
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }
        }

        Label {
            id: colonLbl
            Layout.fillWidth:  true
            textFormat: "MarkdownText"
            text: "# :"
            horizontalAlignment: "AlignHCenter"
        }

        Item {
            Layout.alignment: Qt.AlignLeft
            implicitHeight: _minuteTumbler.implicitHeight
            implicitWidth: _minuteTumbler.implicitWidth

            readonly property real delegateHeight: _minuteTumbler.availableHeight / _minuteTumbler.visibleItemCount

            RoniaTumbler {
                id: _minuteTumbler
                anchors.fill: parent
                currentIndex: 0
                model: 60
            }

            Rectangle {
                x: 12
                y: parent.delegateHeight * 2
                width: parent.width - 24
                height: 2
                color: Style.foreground
            }

            Rectangle {
                x: 12
                y: parent.delegateHeight * 3
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

        Item { }

        RadioButton {
            id: _pmRBtn
            Layout.alignment: Qt.AlignLeft
            text: "PM"
        }
    }

    Label {
        id: msgLabel
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: _contentLay.bottom
            topMargin: 12
        }
        visible: timeProperty === "end-time"
        opacity: isValid ? 0. : 1.
        width: parent.width * 0.8
        font.pointSize: _root.font.pointSize * 0.85
        font.weight: Font.DemiBold
        wrapMode: "Wrap"
        text: "Please, select at least 2 hours period for the schedule"
        horizontalAlignment: "AlignHCenter"

        Behavior on opacity { NumberAnimation { duration: 150 } }
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
        if (editMode && schedule) {
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
