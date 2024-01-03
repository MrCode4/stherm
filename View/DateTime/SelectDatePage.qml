import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

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
    backButtonCallback: function() {
        if (mainStack.currentItem === selectYearItem) {
            mainStack.replace(mainStack.currentItem, dateLay);
        } else {
            if (root.StackView.view) {
                root.StackView.view.pop();
            }
        }
    }

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            dateSelected(selectedDate);

            if (root.StackView.view) {
                root.StackView.view.pop();
            }
        }
    }

    StackView {
        id: mainStack
        anchors.fill: parent
        initialItem: dateLay
    }

    //! Select year grid
    Item {
        id: selectYearItem

        GridView {
            id: yearGrid

            anchors {
                fill: parent
                leftMargin: 16
                rightMargin: 16
                topMargin: 24
                bottomMargin: 24
            }
            clip: true
            model: 101
            boundsBehavior: Flickable.StopAtBounds
            cellWidth: width / 5
            cellHeight: height / 5
            delegate: ItemDelegate {
                width: GridView.view.cellWidth
                height: GridView.view.cellHeight
                highlighted: monthGrid.year === modelData + 1980

                contentItem: Label {
                    text: modelData + 1980
                    font.bold: parent.highlighted
                    color: parent.highlighted ? Style.background : Style.foreground
                    horizontalAlignment: "AlignHCenter"
                    verticalAlignment: "AlignVCenter"
                }

                Component.onCompleted: {
                    background.color = Qt.binding(() => highlighted ? Style.foreground : "transparent")
                }

                onClicked: {
                    monthGrid.year = modelData + 1980;
                    goToDateDelay.start();
                }
            }

            Component.onCompleted: {
                var yearIndx = selectedDate.getFullYear() - 1980;
                yearGrid.positionViewAtIndex(monthGrid.year - 1980, GridView.Center);
            }

            Connections {
                target: monthGrid

                function onYearChanged()
                {
                    yearGrid.positionViewAtIndex(monthGrid.year - 1980, GridView.Contain);
                }
            }
        }

        Timer {
            id: goToDateDelay
            interval: 300
            running: false
            onTriggered: mainStack.replace(mainStack.currentItem, dateLay);
        }
    }

    //! Select date
    ColumnLayout {
        id: dateLay

        readonly property var monthNames: [
            "January", "February", "March", "April", "May", "June",
            "July", "August", "September", "October", "November", "December",
        ]
        readonly property var dayNames:   [
            "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
        ]

        visible: false
        spacing: 8

        //! Selected Date
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: Style.touchTarget * 0.8

            spacing: 12

            Label {
                font.bold: true
                text: selectedDate.toLocaleDateString(locale, "ddd, MMM d yyyy")

                TapHandler {
                    onTapped: {
                        //! Show a grid for selecting year
                        mainStack.replace(mainStack.currentItem, selectYearItem);
                    }
                }
            }

            ToolButton {
                id: goToTodayBtn
                Layout.fillHeight: true
                Layout.preferredWidth: height

                visible: {
                    var today = new Date;
                    return monthGrid.year !== today.getFullYear() && monthGrid.month !== today.getMonth();
                }
                contentItem: RoniaTextIcon {
                    text: FAIcons.arrowRight
                }

                onClicked: {
                    goToToday();
                }
            }
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

    function goToToday()
    {
        var now = new Date();
        monthGrid.year = now.getFullYear();
        monthGrid.month = now.getMonth();

        selectedDate = now;
    }
}
