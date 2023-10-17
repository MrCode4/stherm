import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ScheduleView shows schedules and provide ui for it
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

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
}
