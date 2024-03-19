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

    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property NightMode          nightMode

    //! Schedule time property: end-time or start-time: only considered when schedule is set
    property string             timeProperty:   "start-time"

    //! Start time for schedule if this is end-time
    property date               startTime:      Date

    //! Min minutes diff between start and end time
    readonly property int       minTimeDiff:    120


    //! This shows whether the inputs in this page are valid or not
    readonly property bool      isValid: true

    //! Time in string format: 'hh:mm AM/PM'
    property string    selectedTime

    Component.onCompleted: {
        if (nightMode) {
            var time
            if (timeProperty === "start-time") {
                time = nightMode.startTime;
            } else if (timeProperty === "end-time") {
                time = nightMode.endTime;
            }

            if (time) {
                setTimeFromString(time);
            }
        }
    }

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: AppStyle.size // _contentLay.implicitWidth + leftPadding + rightPadding
    implicitHeight: _contentLay.implicitHeight +
                    + implicitHeaderHeight + implicitFooterHeight + topPadding + bottomPadding
    topPadding: 24
    backButtonVisible: false
    titleHeadeingLevel: 4
    title: timeProperty === "start-time" ? "Start Time" : "End Time"

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing
    ToolButton {
        parent: nightMode ? root.header.contentItem : root
        visible: true
        enabled: isValid
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            saveTime();
        }
    }

    Binding {
        target: root
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

            Tumbler {
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
            textFormat: Text.MarkdownText
            text: "# :"
            horizontalAlignment: Text.AlignHCenter
        }

        Item {
            Layout.alignment: Qt.AlignLeft
            implicitHeight: _minuteTumbler.implicitHeight
            implicitWidth: _minuteTumbler.implicitWidth

            readonly property real delegateHeight: _minuteTumbler.availableHeight / _minuteTumbler.visibleItemCount

            Tumbler {
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

    /* Methods
     * ****************************************************************************************/
    function saveTime()
    {
        if (nightMode) {
            if (timeProperty === "start-time" && nightMode.startTime !== selectedTime) {
                nightMode.startTime = selectedTime;
            } else if (timeProperty === "end-time" && nightMode.endTime !== selectedTime) {
                nightMode.endTime = selectedTime;
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
