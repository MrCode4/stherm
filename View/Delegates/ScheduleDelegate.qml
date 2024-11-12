import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleDelegate provides a delegate for displaying schedules in ScheduleView
 * ***********************************************************************************************/
ItemDelegate {
    id: root

    /* Signals
     * ****************************************************************************************/
    signal sendRemovedRequest()
    signal isScheduleIncomaptible()

    /* Property declaration
     * ****************************************************************************************/
    //! UiSession
    property UiSession              uiSession

    //! SchedulesController
    property SchedulesController    schedulesController: uiSession?.schedulesController ?? null

    //! Schedule
    property ScheduleCPP            schedule

    //! Index in ListView
    property int                    delegateIndex

    /* Object properties
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    QtObject {
        id: internal

        property var overlappingSchedules: []
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 4

        Layout.topMargin: 15
        Layout.leftMargin: 10

        spacing: 10

        RowLayout {
            spacing: 8
            Layout.fillWidth: true
            Layout.topMargin: 0

            //! Schedule icon
            RoniaTextIcon {
                id: scheduleIcon


                width: 24
                font.pointSize: Style.fontIconSize.smallPt
                text: {
                    switch(schedule?.type) {
                    case AppSpec.Away:
                        return "\uf30d";
                    case AppSpec.Night:
                        return "\uf186"
                    case AppSpec.Home:
                        return "\uf015"
                    case AppSpec.Custom:
                        return "\uf1de"
                    }

                    return "-";
                }
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            //! Schedule name
            Label {
                id: scheduleNameLabel

                Layout.fillWidth: true
                font.bold: true
                text: schedule?.name ?? ""
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
            }

            //! Enable switch
            Switch {
                id: _scheduleEnableSw
                checked: schedule?.enable ?? false

                onToggled: {
                    if (uiSession && schedule && schedule.enable !== checked) {
                        if (checked) {
                            //! First check the schedule compability
                            if (schedulesController.isScheduleIncompatible(schedule, uiSession.appModel.systemSetup.systemMode)) {
                                isScheduleIncomaptible();
                                toggle();
                                return;
                            }

                            //! Then find if there is any overlapping schedules
                            internal.overlappingSchedules = schedulesController.findOverlappingSchedules(
                                        schedule.startTime, schedule.endTime,
                                        schedule.repeats, schedule);

                            if (internal.overlappingSchedules.length > 0) {
                                //! First uncheck this Switch
                                toggle() //! This won't emit toggled() signal so no recursion occurs

                                uiSession.popUps.scheduleOverlapPopup.accepted.connect(setActive);
                                uiSession.popUps.scheduleOverlapPopup.rejected.connect(disconnect);
                                uiSession.popupLayout.displayPopUp(uiSession.popUps.scheduleOverlapPopup);
                                return;
                            }
                        }

                        schedule.enable = checked;
                        if (checked) {
                            // Update system mode
                            updateScheduleMode(schedule, uiSession.appModel.systemSetup.systemMode);
                        }

                        uiSession.appModel.schedulesChanged();

                        //Shows a proper toast message upon activation of a schedule
                        if(schedule.enable === true) {
                            var dt = schedulesController.prepareToastMessage(schedule);
                            uiSession.toastManager.showToast(dt.message, dt.detail);
                        }

                        // Send Data to server when a schedule changed...
                        // Edit schedule
                        schedulesController.editScheduleInServer(schedule);
                        uiSession.deviceController.saveSettings();
                    }
                }
            }

            //! Delete button
            ToolButton {
                contentItem: RoniaTextIcon {
                    text: FAIcons.trashCan
                }

                onClicked: {
                    sendRemovedRequest();
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            opacity: 0.8
            spacing: 16

            Label {
                Layout.fillHeight: true

                width: _fontMetric.advanceWidth(" Heating ")
                font.pointSize: Qt.application.font.pointSize * 0.8
                text: {
                    if (schedule.systemMode === AppSpec.Cooling) {
                        return "Cooling";

                    } else if (schedule.systemMode === AppSpec.Heating || schedule.systemMode === AppSpec.EmergencyHeat) {
                        return "Heating";

                    } else if (schedule.systemMode === AppSpec.Auto) {
                        return "Auto";
                    }

                    return "";
                }
            }

            //! Schedule repeats
            Label {
                Layout.fillHeight: true
                Layout.fillWidth: true

                property color disableColor: Qt.alpha(AppStyle.primaryTextColor, 0.3)

                //! Schedule repeats with HTML format based on schedule.repeats.
                property string formattedScheduleRepeats: `<span style='color:${Boolean(schedule.repeats.includes("Mo")) ? AppStyle.primaryTextColor : disableColor};'>M</span> ` +
                                      `<span style='color:${Boolean(schedule.repeats.includes("Tu")) ? AppStyle.primaryTextColor : disableColor};'>T</span> ` +
                                      `<span style='color:${Boolean(schedule.repeats.includes("We")) ? AppStyle.primaryTextColor : disableColor};'>W</span> ` +
                                      `<span style='color:${Boolean(schedule.repeats.includes("Th")) ? AppStyle.primaryTextColor : disableColor};'>T</span> ` +
                                      `<span style='color:${Boolean(schedule.repeats.includes("Fr")) ? AppStyle.primaryTextColor : disableColor};'>F</span> ` +
                                      `<span style='color:${Boolean(schedule.repeats.includes("Sa")) ? AppStyle.primaryTextColor : disableColor};'>S</span> ` +
                                      `<span style='color:${Boolean(schedule.repeats.includes("Su")) ? AppStyle.primaryTextColor : disableColor};'>S</span> `;
                font: _fontMetric.font
                text: formattedScheduleRepeats
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignRight
            }

            //! Schedule time
            Label {
                Layout.fillHeight: true

                horizontalAlignment: Text.AlignRight
                font.pointSize: Qt.application.font.pointSize * 0.75
                text: `${schedule.startTime} - ${schedule.endTime}`
            }
        }
    }

    FontMetrics {
        id: _fontMetric
        font.pointSize: root.font.pointSize * 0.8
    }

    ParallelAnimation {
        id: _removeAnima
        running: false
        loops: 1

        NumberAnimation {
            target: root
            property: "opacity"
            to: 0
            duration: 200
        }

        NumberAnimation {
            target: root
            property: "x"
            to: -root.width
            duration: 200
        }

        onFinished: {
            if (schedulesController) {
                schedulesController.removeSchedule(schedule);
            }
        }
    }

    function setActive()
    {
        //! If there is overlapping Schedules disable them
        internal.overlappingSchedules.forEach((element, index) => {
                                                  element.enable = false;
                                                  schedulesController.editScheduleInServer(element);
                                              });

        disconnect()

        if (schedule?.enable === false) {
            schedule.enable = true;
            // Update system mode
            updateScheduleMode(schedule, uiSession.appModel.systemSetup.systemMode);

            // Send Data to server when a schedule changed...
            // Edit schedule
            schedulesController.editScheduleInServer(schedule);
            uiSession.deviceController.saveSettings();

            //Shows a proper toast message upon activation of a schedule
            var dt = schedulesController.prepareToastMessage(schedule);
            uiSession.toastManager.showToast(dt.message, dt.detail);
        }

        uiSession.appModel.schedulesChanged();
    }

    //! Disconnect the popUps
    function disconnect() {
        uiSession.popUps.scheduleOverlapPopup.accepted.disconnect(setActive);
        uiSession.popUps.scheduleOverlapPopup.rejected.disconnect(disconnect);
    }

    //! The remove request accepted by the parent
    function removeRequestAccepted() {
        //! Remove this item
        _removeAnima.running = true;
    }

    //! update schedule Mode based on SystemMode if in one of cooling, heating, Auto or EmergencyHeat Modes
    function updateScheduleMode(schedule, systemMode) {
        if (systemMode === AppSpec.Cooling ||
                systemMode === AppSpec.Heating ||
                systemMode === AppSpec.EmergencyHeat ||
                systemMode === AppSpec.Auto)
            schedulesController.setSchduleMode(schedule, systemMode);
    }
}

