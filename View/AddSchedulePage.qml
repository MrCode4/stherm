import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! schedulesModel: use to create new Schedule instance
    property SchedulesController     schedulesController: uiSession?.schedulesController ?? null
    //! Device reference
    property I_Device                device:              uiSession?.appModel ?? null

    readonly property ScheduleCPP    defaultSchedule:     AppSpec.getDefaultSchedule(_internal.newSchedule.type);

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
        id: nxtConfBtn
        parent: root.header.contentItem
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
                            Date.fromLocaleTimeString(Qt.locale(), _internal.newSchedule.endTime, "hh:mm AP"),
                            _internal.newSchedule.repeats);

                if (_internal.overlappingSchedules.length > 0) {
                    //! New schedules overlapps with at least one other Schedule
                    uiSession.popUps.scheduleOverlapPopup.accepted.connect(saveEnabledSchedule);
                    uiSession.popUps.scheduleOverlapPopup.rejected.connect(saveDisabledSchedule);
                    uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);
                } else {
                    saveSchedule();
                }
            } else {

                // if sensors are empty we skip this page!
                if (_newSchedulePages.currentItem instanceof ScheduleRepeatPage) {
                    if (device?._sensors.length === 0) {
                        _internal.newSchedule.dataSource = "Onboard Sensor";
                        _newSchedulePages.push(_preivewPage)
                        return;
                    }
                }

                //! Go to next page
                _newSchedulePages.push(_newSchedulePages.currentItem.nextPage)
            }
        }
    }

    //! StackView for new-schedule pages
    StackView {
        id: _newSchedulePages
        anchors.fill: parent

        initialItem: _sheduleNamePage
    }

    QtObject {
        id: _internal

        property ScheduleCPP newSchedule: ScheduleCPP { }

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

            onAccepted: {
                nxtConfBtn.forceActiveFocus();
                nxtConfBtn.clicked();
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

            uiSession: root.uiSession

            schedule: root.defaultSchedule

            onTempratureChanged: {
                if (temprature !== _internal.newSchedule.temprature) {
                    _internal.newSchedule.temprature = temprature;
                }
            }

            Component.onCompleted: {
                if (_internal.newSchedule.type === AppSpec.Custom) {
                    schedule.temprature = appModel.requestedTemp;
                }
            }
        }
    }

    Component {
        id: _startTimePage

        ScheduleTimePage {
            readonly property Component nextPage: _endTimePage

            title: "Start Time"

            schedule: root.defaultSchedule

            onSelectedTimeChanged: {
                if (isValid && selectedTime !== _internal.newSchedule.startTime) {
                    _internal.newSchedule.startTime = selectedTime;
                }
            }

            Component.onCompleted: {
                if (_internal.newSchedule.type === AppSpec.Custom) {
                    //! Set start time to current time
                    setTimeFromString((new Date).toLocaleTimeString(Qt.locale(), "hh:mm AP"));
                }
            }
        }
    }

    Component {
        id: _endTimePage

        ScheduleTimePage {
            readonly property Component nextPage: _repeatPage

            title: "End Time"
            timeProperty: "end-time"

            startTime: Date.fromLocaleTimeString(Qt.locale(), _internal.newSchedule.startTime, "hh:mm AP")
            schedule: root.defaultSchedule

            onSelectedTimeChanged: {
                if (isValid && selectedTime !== _internal.newSchedule.endTime) {
                    _internal.newSchedule.endTime = selectedTime;
                }
            }

            Component.onCompleted: {
                if (_internal.newSchedule.type === AppSpec.Custom) {
                    //! Set selected time to 2 hours after schedule's start time
                    var endTime = Date.fromLocaleTimeString(locale, _internal.newSchedule.startTime, "hh:mm AP");
                    endTime.setTime(endTime.getTime() + 2 * 1000 * 60 * 60);

                    setTimeFromString(endTime.toLocaleTimeString(locale, "hh:mm AP"));
                }
            }
        }
    }

    Component {
        id: _repeatPage

        ScheduleRepeatPage {
            readonly property Component nextPage: _dataSourcePageCompo

            schedule: root.defaultSchedule

            onRepeatsChanged: {
                if (repeats !== _internal.newSchedule.repeats) {
                    _internal.newSchedule.repeats = repeats;
                }
            }
        }
    }

    Component {
        id: _dataSourcePageCompo

        ScheduleDataSourcePage {
            readonly property Component nextPage: _preivewPage

            uiSession: root.uiSession
            onSensorChanged: {
                _internal.newSchedule.dataSource = (sensor?.name ?? "");
            }
        }
    }

    Component {
        id: _preivewPage

        SchedulePreviewPage {
            readonly property Component nextPage: null

            uiSession: root.uiSession
            schedule: _internal.newSchedule
        }
    }

    /* Methods
     * ****************************************************************************************/
    //! Saves a schedule as disabled, do not disabled overlapping schedules if any
    function saveDisabledSchedule()
    {
        _internal.newSchedule.enable = false;

        saveScheduleAndDisconnect();
    }

    //! Saves a schedule as enabled, disable overlapping schedules if any
    function saveEnabledSchedule()
    {
        //! If there is overlapping Schedules disable them
        _internal.overlappingSchedules.forEach((element, index) => {
                                                   element.enable = false;
                                               });
        saveScheduleAndDisconnect();
    }

    function saveScheduleAndDisconnect()
    {
        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(saveEnabledSchedule);
        uiSession.popUps.scheduleOverlapPopup.rejected.disconnect(saveDisabledSchedule);

        saveSchedule();
    }

    function saveSchedule()
    {
        if (schedulesController) {
            schedulesController.saveNewSchedule(_internal.newSchedule);
        }
        deviceController.finalizeSettings();

        if (root.StackView.view) {
            root.StackView.view.pop();
        }
    }
}
