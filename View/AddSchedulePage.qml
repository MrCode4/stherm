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
                            _internal.newSchedule.startTime, _internal.newSchedule.endTime,
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
                if (_newSchedulePages.currentItem instanceof ScheduleTempraturePage) {
                    if (device?._sensors.length === 0) {
                        _internal.newSchedule.dataSource = "Onboard Sensor";
                        _newSchedulePages.push(_preivewPage)
                        return;
                    }
                } else if (_newSchedulePages.currentItem instanceof ScheduleNamePage) {
                    _newSchedulePages.currentItem.updateModel();
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

            onUpdateModel: {
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
            readonly property Component nextPage:_startEndTimePage

            onTypeChanged: {
                if (type !== _internal.newSchedule.type) {
                    _internal.newSchedule.type = type;
                }
            }
        }
    }

    Component {
        id: _startEndTimePage

        ScheduleTimePage {
            readonly property Component nextPage: _repeatPage

            schedule: root.defaultSchedule

            onSelectedStartTimeChanged: {
                saveStartTime();
            }

            onSelectedEndTimeChanged: {
               saveEndTime();
            }

            onIsValidChanged: {
                saveStartTime();
                saveEndTime();
            }

            function saveStartTime() {
                if (isValid) {
                    if (selectedStartTime !== _internal.newSchedule.startTime) {
                        _internal.newSchedule.startTime = selectedStartTime;
                    }
                }
            }

            function saveEndTime() {
                if (isValid) {
                    if (selectedEndTime !== _internal.newSchedule.endTime) {
                        _internal.newSchedule.endTime = selectedEndTime;
                    }
                }
            }
        }
    }

    Component {
        id: _repeatPage

        ScheduleRepeatPage {
            readonly property Component nextPage: _temperaturePage

            schedule: root.defaultSchedule

            onRepeatsChanged: {
                if (repeats !== _internal.newSchedule.repeats) {
                    _internal.newSchedule.repeats = repeats;
                }
            }
        }
    }

    Component {
        id: _temperaturePage

        ScheduleTempraturePage {
            // Move to enable/disable page
            readonly property Component nextPage:  _dataSourcePageCompo

            uiSession: root.uiSession

            schedule: root.defaultSchedule

            onMinimumTemperatureChanged: {
                if (minimumTemperature !== _internal.newSchedule.minimumTemperature) {
                    _internal.newSchedule.minimumTemperature = minimumTemperature;
                }
            }

            onMaximumTemperatureChanged: {
                if (maximumTemperature !== _internal.newSchedule.maximumTemperature) {
                    _internal.newSchedule.maximumTemperature = maximumTemperature;
                }
            }

            Component.onCompleted: {
                if (_internal.newSchedule.type === AppSpec.Custom) {
                    schedule.minimumTemperature = appModel.autoMinReqTemp;
                    schedule.maximumTemperature = appModel.autoMaxReqTemp;
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

            //If the schedule is enabled, show a proper toast message
            if(_internal.newSchedule.enable === true)
                var dt = schedulesController.prepareToastMessage(_internal.newSchedule);
                uiSession.toastManager.showToast(dt.message, dt.detail);
        }

        deviceController.updateEditMode(AppSpec.EMSchedule);
        deviceController.saveSettings();

        if (root.StackView.view) {
            root.StackView.view.pop();
        }
    }
}
