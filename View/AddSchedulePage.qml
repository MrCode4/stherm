import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! schedulesModel: use to create new Schedule instance
    property SchedulesModel     schedulesModel: uiSession.appModel?.schedulesModel ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Add New Schedule"
    footer: RowLayout {
        ToolButton {
            visible: _newSchedulePages.depth > 1
            contentItem: RoniaTextIcon {
                text: "\uf060"
            }

            onClicked: {
                _newSchedulePages.pop();
            }
        }
    }
    backButtonTextIcon: _newSchedulePages.depth > 1 ? "\uf00d" : "\uf060"

    /* Children
     * ****************************************************************************************/
    //! Next/Confirm button
    ToolButton {
        parent: _root.header

        RoniaTextIcon {
            anchors.centerIn: parent
            opacity: _newSchedulePages.currentItem?.nextPage ? 1. : 0.
            text: "\uf061"
        }

        RoniaTextIcon {
            opacity: _newSchedulePages.currentItem?.nextPage ? 0. : 1.
            anchors.centerIn: parent
            text: "\uf00c"
        }

        onClicked: {
            if (!_newSchedulePages.currentItem.nextPage) {
                //! It's done, save schedule and go back
                if (schedulesModel) {
                    schedulesModel.saveNewSchedule(_internal.newSchedule);
                }

                if (_root.StackView.view) {
                    _root.StackView.view.pop();
                }
            } else {
                //! Go to next page
                _newSchedulePages.push(_newSchedulePages.currentItem.nextPage)
            }
        }
    }

    //! StackView for new-schedule pages
    StackView {
        id: _newSchedulePages
        anchors.centerIn: parent
        implicitHeight: Math.min(parent.height, currentItem?.implicitHeight)
        implicitWidth: Math.min(parent.width, currentItem?.implicitWidth)

        initialItem: _sheduleNamePage
    }

    QtObject {
        id: _internal

        property Schedule newSchedule: Schedule { }
    }

    //! Page Components
    Component {
        id: _sheduleNamePage

        ScheduleNamePage {
            readonly property Component nextPage: _typePage

            onScheduleNameChanged: {
                if (_internal.newSchedule.name !== scheduleName) {
                    _internal.newSchedule.name = scheduleName;
                }
            }
        }
    }

    Component {
        id: _typePage

        ScheduleTypePage {
            readonly property Component nextPage: _tempraturePage

            onTypeChanged: {
                if (type !== _internal.newSchedule.type) {
                    _internal.newSchedule.type = type;
                }
            }
        }
    }

    Component {
        id: _tempraturePage

        ScheduleTempraturePage {
            readonly property Component nextPage: _startTimePage

            onTempratureChanged: {
                if (temprature !== _internal.newSchedule.temprature) {
                    _internal.newSchedule.temprature = temprature;
                }
            }
        }
    }

    Component {
        id: _startTimePage

        ScheduleTimePage {
            readonly property Component nextPage: _endTimePage

            title: "Start Time"
            onSelectedTimeChanged: {
                if (selectedTime !== _internal.newSchedule.startTime) {
                    _internal.newSchedule.startTime = selectedTime;
                }
            }
        }
    }

    Component {
        id: _endTimePage

        ScheduleTimePage {
            readonly property Component nextPage: _repeatPage

            title: "End Time"
            onSelectedTimeChanged: {
                if (selectedTime !== _internal.newSchedule.endTime) {
                    _internal.newSchedule.endTime = selectedTime;
                }
            }
        }
    }

    Component {
        id: _repeatPage

        ScheduleRepeatPage {
            readonly property Component nextPage: _preivewPage

            onRepeatsChanged: {
                if (repeats.toString() !== _internal.newSchedule.repeats.toString()) {
                    _internal.newSchedule.repeats = repeats;
                }
            }
        }
    }

    Component {
        id: _preivewPage

        SchedulePreviewPage {
            readonly property Component nextPage: null

            schedule: _internal.newSchedule
        }
    }
}
