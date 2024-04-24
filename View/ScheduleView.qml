import QtQuick
import QtQuick.Layouts

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

    Component.onCompleted: deviceController.updateEditMode(AppSpec.EMSchedule);
    Component.onDestruction: deviceController.updateEditMode(AppSpec.EMSchedule, false);

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
            required property var modelData
            required property int index

            width: ListView.view.width
            height: Style.delegateHeight
            uiSession: _root.uiSession
            schedule: (modelData instanceof ScheduleCPP) ? modelData : null
            delegateIndex: index

            onRemoved: {
                if (schedulesController) {
                    schedulesController.removeSchedule(schedule);
                }
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
}
