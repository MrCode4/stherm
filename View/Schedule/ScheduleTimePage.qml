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
    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property Schedule   schedule

    //! Schedule time property: end-time or start-time: only considered when schedule is set
    property string     timeProperty: "star-time"

    //! Time in string format: 'hh:mm AM/PM'
    readonly property string selectedTime: {
        var h = `${_hourTumbler.currentItem.modelData}`;
        var m = `${_minuteTumbler.currentItem.modelData}`;

        if (h.length === 1) h = "0" + h;
        if (m.length === 1) m = "0" + m;

        return `${h}:${m} ${_amRBtn.checked ? "AM" : "PM"}`;
    }

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: _contentLay.implicitWidth + leftPadding + rightPadding
    implicitHeight: _contentLay.implicitHeight + implicitHeaderHeight + implicitFooterHeight + topPadding + bottomPadding
    topPadding: 24
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
    GridLayout {
        id: _contentLay
        anchors.centerIn: parent

        columns: 3
        columnSpacing: 12
        rowSpacing: 32

        Tumbler {
            id: _hourTumbler
            model: Array.from({ length: 12 }, (elem, indx) => indx + 1)
        }

        Label {
            textFormat: "MarkdownText"
            text: "# :"
            Layout.fillWidth:  true
        }

        Tumbler {
            id: _minuteTumbler
            model: 60
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

    onSelectedTimeChanged: {
        if (schedule) {
            if (timeProperty === "start-time" && schedule.startTime !== selectedTime) {
                schedule.startTime = selectedTime;
            } else if (timeProperty === "end-time" && schedule.endTime !== selectedTime) {
                schedule.endTime = selectedTime;
            }
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
                _hourTumbler.currentIndex = Number(time.slice(0, 2)) - 1;
                _minuteTumbler.currentIndex = Number(time.slice(3, 5));
                if (time.slice(6, 8) === "AM") {
                    _amRBtn.checked = true;
                } else if (time.slice(6, 8) === "PM") {
                    _pmRBtn.checked = true;
                }
            }
        }
    }
}
