import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm
import "."
/*! ***********************************************************************************************
 * SelectDatePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal dateSelected(date date)

    /* Property Declaration
     * ****************************************************************************************/
    //! Selected Date
    property date selectedDate: new Date

    /* Object properties
     * ****************************************************************************************/
    topPadding: 0
    bottomPadding: 4
    title: "Select Date"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: dateSelected(selectedDate)
    }

    ColumnLayout {
        id: dateLay

        readonly property var monthNames: [
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December",
        ]
        readonly property var dayNames:   [
            "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
        ]

        anchors.fill: parent
        spacing: 8

        //! Selected Date
        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.bottomMargin: 8
            font.bold: true
            text: selectedDate.toLocaleDateString(locale, "ddd, MMM d yyyy")
        }

        Rectangle {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: root.width / 1.8
            Layout.preferredHeight: 1
            Layout.bottomMargin: 8
        }

        DayOfWeekRow {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.bottomMargin: 8
            Layout.leftMargin: 16
            Layout.rightMargin: Layout.leftMargin
            locale: Qt.locale()
            delegate: Label {
                opacity: 0.6
                font.bold: false
                text: model.narrowName
                horizontalAlignment: "AlignHCenter"
            }
        }

        MonthGrid {
            id: monthGrid

            property Item currentDateDelegate: null

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: Layout.leftMargin
            Layout.alignment: Qt.AlignCenter
            delegate: MonthDayDelegate {
                enabled: dayModel.month === monthGrid.month
                dayModel: model
                highlighted: monthGrid.year === selectedDate.getFullYear()
                             && dayModel.month === selectedDate.getMonth()
                             && dayModel.day === selectedDate.getDate()

                onClicked: selectedDate = new Date(monthGrid.year, monthGrid.month, dayModel.day);
            }
        }

        //! Current Month
        RowLayout {
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignCenter

            ToolButton {
                contentItem: RoniaTextIcon {
                    font.pointSize: Style.fontIconSize.smallPt
                    text: "\uF053"
                }

                onClicked: {
                    var date = new Date(monthGrid.year, monthGrid.month);
                    date.setMonth(date.getMonth() - 1);

                    monthGrid.year = date.getFullYear();
                    monthGrid.month = date.getMonth();
                }
            }

            Label {
                font.bold: true
                text: `${dateLay.monthNames[monthGrid.month]} ${monthGrid.year}`
            }

            ToolButton {
                contentItem: RoniaTextIcon {
                    font.pointSize: Style.fontIconSize.smallPt
                    text: "\uF054"
                }

                onClicked: {
                    var date = new Date(monthGrid.year, monthGrid.month);
                    date.setMonth(date.getMonth() + 1);

                    monthGrid.year = date.getFullYear();
                    monthGrid.month = date.getMonth();
                }
            }
        }
    }
}
