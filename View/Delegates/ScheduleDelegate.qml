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
                        //! First find if there is any overlapping schedules
                        if (checked) {

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
                            schedule.systemMode = uiSession.appModel.systemSetup.systemMode;
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
                    text: "\uf2ed"
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

                    } else if (schedule.systemMode === AppSpec.Heating) {
                        return "Heating";

                    }

                    return "Auto";
                }
            }

            //! Schedule repeats
            Label {
                Layout.fillHeight: true
                Layout.fillWidth: true

                property var scheduleRepeats: schedule?.repeats.length > 0 ? schedule.repeats.split(",").map(day => day.charAt(0)) : ["No repeat"]

                font: _fontMetric.font
                text: scheduleRepeats.join(" ")
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
            schedule.systemMode = uiSession.appModel.systemSetup.systemMode;

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
}

