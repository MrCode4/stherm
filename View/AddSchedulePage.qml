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
    property SchedulesController     schedulesController: uiSession?.schedulesController ?? null

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
        parent: _root.header.contentItem
        enabled: !_newSchedulePages.currentItem?.nextPage || _newSchedulePages.currentItem?.isValid

        RoniaTextIcon {
            anchors.centerIn: parent
            opacity: _newSchedulePages.currentItem?.nextPage ? 1. : 0.
            text: FAIcons.arrowRight
        }

        RoniaTextIcon {
            opacity: _newSchedulePages.currentItem?.nextPage ? 0. : 1.
            anchors.centerIn: parent
            text: FAIcons.check
        }

        onClicked: {
            if (!_newSchedulePages.currentItem.nextPage) {
                //! It's done, save schedule and go back
                //! First check if this schedule has overlap with other Schedules
                _internal.overlappingSchedules = schedulesController.findOverlappingSchedules(
                            Date.fromLocaleTimeString(Qt.locale(), _internal.newSchedule.startTime, "hh:mm AP"),
                            Date.fromLocaleTimeString(Qt.locale(), _internal.newSchedule.endTime, "hh:mm AP"));

                if (_internal.overlappingSchedules.length > 0) {
                    //! New schedules overlapps with at least one other Schedule
                    uiSession.popUps.scheduleOverlapPopup.accepted.connect(saveSchedule);
                    uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);
                } else {
                    saveSchedule();
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
        width: _root.availableWidth

        initialItem: _sheduleNamePage
    }

    QtObject {
        id: _internal

        property Schedule newSchedule: Schedule { }

        property var overlappingSchedules: []
    }

    //! Page Components
    Component {
        id: _sheduleNamePage

        ScheduleNamePage {
            readonly property Component nextPage: _typePage

            onScheduleNameChanged: {
                if (isValid &&_internal.newSchedule.name !== scheduleName) {
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

            uiSession: _root.uiSession

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
                if (isValid && selectedTime !== _internal.newSchedule.startTime) {
                    _internal.newSchedule.startTime = selectedTime;
                }
            }
        }
    }

    Component {
        id: _endTimePage

        ScheduleTimePage {
            readonly property Component nextPage: _repeatPage

            timeProperty: "end-time"
            startTime: Date.fromLocaleTimeString(Qt.locale(), _internal.newSchedule.startTime, "hh:mm AP")
            title: "End Time"
            onSelectedTimeChanged: {
                if (isValid && selectedTime !== _internal.newSchedule.endTime) {
                    _internal.newSchedule.endTime = selectedTime;
                }
            }
        }
    }

    Component {
        id: _repeatPage

        ScheduleRepeatPage {
            readonly property Component nextPage: dataSourcePageCompo

            onRepeatsChanged: {
                if (repeats.toString() !== _internal.newSchedule.repeats.toString()) {
                    _internal.newSchedule.repeats = repeats;
                }
            }
        }
    }

    Component {
        id: dataSourcePageCompo

        ScheduleDataSourcePage {
            readonly property Component nextPage: _preivewPage

            uiSession: _root.uiSession
            onSensorChanged: {
                _internal.newSchedule.dataSource = (sensor?.name ?? "");
            }
        }
    }

    Component {
        id: _preivewPage

        SchedulePreviewPage {
            readonly property Component nextPage: null

            uiSession: _root.uiSession
            schedule: _internal.newSchedule
        }
    }

    /* Methods
     * ****************************************************************************************/
    function saveSchedule()
    {
        //! If there is overlapping Schedules disable them
        _internal.overlappingSchedules.forEach((element, index) => {
                                                   element.active = false;
                                               });

        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(saveSchedule);

        if (schedulesController) {
            schedulesController.saveNewSchedule(_internal.newSchedule);
        }

        if (_root.StackView.view) {
            _root.StackView.view.pop();
        }
    }
}
