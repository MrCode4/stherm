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
    //! ScheduleController instance
    property ScheduleController     scheduleController: uiSession?.scheduleController

    /* Object properties
     * ****************************************************************************************/
    title: "Schedule"

    /* Children
     * ****************************************************************************************/
    //! Add schedule button -> add it to header
    ToolButton {
        parent: _root.header //! Which is a RowLayout
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
        anchors.fill: parent
        clip: true
        model: scheduleController?.schedules
        delegate: ScheduleDelegate {
            required property var modelData
            required property int index

            width: ListView.view.width
            height: Material.delegateHeight
            schedule: modelData
            delegateIndex: index

            onRemoved: {
                if (scheduleController) {
                    scheduleController.removeSchedule(schedule);
                }
            }
        }
    }

    FontMetrics {
        id: _fontMetric
    }
}
