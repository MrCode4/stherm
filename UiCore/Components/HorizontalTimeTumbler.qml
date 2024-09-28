import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleStartTimePage provides ui for selecting start time in AddSchedulePage
 * ***********************************************************************************************/

Pane {

    readonly property string    selectedTime:   {
        var h = `${_hourTumbler.currentItem.modelData}`;
        var m = `${_minuteTumbler.currentItem.modelData}`;

        if (h.length === 1) h = "0" + h;
        if (m.length === 1) m = "0" + m;

        return `${h}:${m} ${_amRBtn.checked ? "AM" : "PM"}`;
    }

    RowLayout {
        anchors.fill: parent

        spacing: 80

        ColumnLayout {
            id:apLayout

            spacing: 0

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

        RowLayout {
            spacing: 0

            //! Hours Tumbler
            Item {
                Layout.alignment: Qt.AlignCenter
                implicitHeight: _hourTumbler.implicitHeight
                implicitWidth: _hourTumbler.implicitWidth

                readonly property real delegateHeight: _hourTumbler.availableHeight / _hourTumbler.visibleItemCount


                RoniaTumbler {
                    id: _hourTumbler

                    visibleItemCount: 3
                    anchors.fill: parent
                    currentIndex: 0
                    model: Array.from({ length: 12 }, (elem, indx) => indx + 1)
                    tumblerViewHeight: 120
                }

                Rectangle {
                    x: 12
                    y: parent.delegateHeight
                    width: parent.width - 24
                    height: 2
                    color: Style.foreground
                }

                Rectangle {
                    x: 12
                    y: parent.delegateHeight * 2
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
                horizontalAlignment: Qt.AlignHCenter
            }

            //! Minutes Tumbler
            Item {
                Layout.alignment: Qt.AlignLeft
                implicitHeight: _minuteTumbler.implicitHeight
                implicitWidth: _minuteTumbler.implicitWidth

                readonly property real delegateHeight: _minuteTumbler.availableHeight / _minuteTumbler.visibleItemCount

                RoniaTumbler {
                    id: _minuteTumbler
                    anchors.fill: parent
                    visibleItemCount: 3
                    currentIndex: 0
                    model: 60
                    tumblerViewHeight: 120
                }

                Rectangle {
                    x: 12
                    y: parent.delegateHeight
                    width: parent.width - 24
                    height: 2
                    color: Style.foreground
                }

                Rectangle {
                    x: 12
                    y: parent.delegateHeight * 2
                    width: parent.width - 24
                    height: 2
                    color: Style.foreground
                }
            }
        }
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
