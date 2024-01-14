import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SetTimePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal timeSelected(date time)

    /* Property declaration
     * ****************************************************************************************/
    //! Is 12 hour format
    readonly property bool is12Hour: appModel?.setting?.timeFormat === AppSpec.TimeFormat.Hour12

    /* Object properties
     * ****************************************************************************************/
    title: "Set System Time"

    /* Children
     * ****************************************************************************************/
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            var timeString = hourTumbler.hour + ":" + minuteTumbler.minute + ":00";
            timeString += (is12Hour ? (_amRBtn.checked ? " AM" : " PM") : "");

            const timeFormat = is12Hour ? "hh:mm:ss AP" : "hh:mm:ss";

            var selectedTime = new Date;
            selectedTime = Date.fromLocaleTimeString(Qt.locale(), timeString, timeFormat);
            selectedTime.setSeconds(0);
            timeSelected(selectedTime);

            if (root.StackView.view) {
                root.StackView.view.pop();
            }
        }
    }

    ButtonGroup {
        buttons: [_amRBtn, _pmRBtn]
    }

    RowLayout {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -16
        spacing: 32

        ColumnLayout {
            spacing: 16
            Tumbler {
                id: hourTumbler

                property string hour: {
                    var currentText = currentItem.modelData.toString();
                    return currentText.length === 1 ? "0" + currentText : currentText;
                }

                Layout.preferredHeight: 6 * contentItem.delegateHeight
                Layout.preferredWidth: 96
                Layout.alignment: Qt.AlignCenter
                model: is12Hour ? Array.from({ length: 12 }, (elem, indx) => indx + 1) : 24
                font.pointSize: metrics.font.pointSize

                Component.onCompleted: contentItem.delegateHeight = metrics.height * 1.8
            }

            //! AM and PM radio buttons
            RadioButton {
                id: _amRBtn
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                visible: is12Hour
                text: "AM"
                checked: true
            }
        }

        ColumnLayout {
            spacing: 16
            Tumbler {
                id: minuteTumbler

                property string minute: (currentIndex < 10 ? "0" : "") + currentIndex

                Layout.preferredHeight: 6 * contentItem.delegateHeight
                Layout.preferredWidth: 96
                Layout.alignment: Qt.AlignCenter
                model: 60
                font.pointSize: metrics.font.pointSize

                Component.onCompleted: contentItem.delegateHeight = metrics.height * 1.8
            }

            RadioButton {
                id: _pmRBtn
                Layout.fillWidth: true
                visible: is12Hour
                text: "PM"
            }
        }
    }

    TextMetrics {
        id: metrics
        font.pointSize: root.font.pointSize * 1.4
        text: "99"
    }

    Component.onCompleted: {
        var now = DateTimeManager.now;
        if (is12Hour) {
            var nowStr = now.toLocaleTimeString(locale, "hh:mm AP");
            hourTumbler.currentIndex = Number(nowStr.slice(0, 2)) - 1;
            minuteTumbler.currentIndex = Number(nowStr.slice(3, 5));

            var isAm = nowStr.slice(-2) === "AM";
            if (isAm) {
                _amRBtn.checked = true;
            } else {
                _pmRBtn.checked = true;
            }
        } else {
            hourTumbler.currentIndex = now.getHours();
            minuteTumbler.currentIndex = now.getMinutes();
        }
    }
}
