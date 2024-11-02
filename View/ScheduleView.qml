import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleView shows schedules and provide ui for it
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! schedules instance: use to show and remove schedule objects
    property SchedulesController     schedulesController: uiSession?.schedulesController

    /* Object properties
     * ****************************************************************************************/
    title: "Schedule"

    Component.onCompleted: {
       schedulesController.lockScheduleFetching(true);
    }

    Component.onDestruction: {
       schedulesController.lockScheduleFetching(false);
    }

    /* Children
     * ****************************************************************************************/
    //! Add schedule button -> add it to header
    ToolButton {
        parent: _root.header.contentItem //! Which is a RowLayout
        contentItem: RoniaTextIcon {
            text: "\ue197"
        }

        onClicked: {
            //! Open adding a schedule page
            if (_root.StackView.view) {
                _root.StackView.view.push("qrc:/Stherm/View/AddSchedulePage.qml", {
                                              "uiSession": uiSession
                                          });
            }
        }
    }

    //! Contents should be a list of current schedules
    ListView {
        ScrollIndicator.vertical: ScrollIndicator {
            x: parent.width - width - 4
            y: _root.contentItem.y
            parent: _root
            height: _root.contentItem.height - 16
        }

        anchors.fill: parent
        anchors.rightMargin: 4
        clip: true
        model: appModel?.schedules ?? []
        delegate: ScheduleDelegate {
            id: scheduleDelegate

            required property var modelData
            required property int index

            width: ListView.view.width
            height: Style.delegateHeight * 1.7
            uiSession: _root.uiSession
            schedule: (modelData instanceof ScheduleCPP) ? modelData : null
            delegateIndex: index

            onSendRemovedRequest: {
                deleteScheduleConfirmPopup.scheduleDelegateToDelete = scheduleDelegate;
                deleteScheduleConfirmPopup.open();

            }

            onIsScheduleIncomaptible: {
                scheduleSystemModeErrorPopup.schedule = schedule;
                uiSession.popupLayout.displayPopUp(scheduleSystemModeErrorPopup);
            }

            onClicked: {
                if (_root.StackView.view) {
                    _root.StackView.view.push(schedulePreview, {
                                                  "schedule": schedule
                                              });
                }
            }
        }
    }

    Component {
        id: schedulePreview

        SchedulePreviewPage {
            uiSession: _root.uiSession
            backButtonVisible: true
            isEditable: true
        }
    }

    //! Use a ConfirmPopup for delete schedules.
    property ConfirmPopup deleteScheduleConfirmPopup: ConfirmPopup {

        property ScheduleDelegate scheduleDelegateToDelete: null
        property ScheduleCPP scheduleToDelete: scheduleDelegateToDelete?.schedule ?? null

        message: "Delete the Schedule?"
        detailMessage: `Are you sure you want to delete ${scheduleToDelete?.name ?? ""}?`
        applyText: qsTr("Delete")

        visible: false
        icon: FAIcons.trashCan
        iconWeight: FAIcons.Light
        buttons: MessageDialog.Cancel | MessageDialog.Apply

        onButtonClicked: button => {
                             if (button === MessageDialog.Apply) {
                                 scheduleDelegateToDelete.removeRequestAccepted();
                             }
                         }
    }

    property ScheduleSystemModeErrorPopup scheduleSystemModeErrorPopup: ScheduleSystemModeErrorPopup {

        onDuplicateSchedule: schedule => {
                                 if (_root.StackView.view) {
                                     var clonedSchedule = schedulesController.saveNewSchedule(schedule);
                                     clonedSchedule.enable = false;

                                     //! Find the proper name for schedule.
                                     var scheduleName = clonedSchedule.name;
                                     var num = 1;
                                     while (schedulesController.isScheduleNameExist(scheduleName)) {
                                         scheduleName += `_${num}`;
                                         num++;
                                     }

                                      clonedSchedule.name = scheduleName;

                                     if (_root.StackView.view) {
                                         _root.StackView.view.push(schedulePreview, {
                                                                       "schedule": clonedSchedule
                                                                   });
                                     }
                                 }

        }

    }
}
