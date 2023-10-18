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

        Item { }

        RadioButton {
            id: _pmRBtn
            Layout.alignment: Qt.AlignCenter
            text: "PM"
        }
    }
}
