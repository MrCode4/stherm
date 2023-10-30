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
        model: appModel?.schedules ?? []
        delegate: ScheduleDelegate {
            required property var modelData
            required property int index

            width: ListView.view.width
            height: Material.delegateHeight
            schedule: (modelData instanceof Schedule) ? modelData : null
            delegateIndex: index

            onRemoved: {
                if (schedulesController) {
                    schedulesController.removeSchedule(schedule);
                }
            }
        }
    }

    FontMetrics {
        id: _fontMetric
    }
}
