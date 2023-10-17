import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Stherm
import "./Schedule"

/*! ***********************************************************************************************
 * AddSchedulePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Add New Schedule"

    /* Children
     * ****************************************************************************************/
    //! Next/Confirm button
    ToolButton {
        parent: _root.header

        RoniaTextIcon {
            anchors.centerIn: parent
            text: "\uf061"
        }

        onClicked: {

        }
    }

    //! StackView for new-schedule pages
    StackView {
        id: _newSchedulePages
        anchors.centerIn: parent
        implicitHeight: currentItem?.implicitHeight
        implicitWidth: currentItem?.implicitWidth

        initialItem: _typePage
    }

    //! Page Components
    Component {
        id: _typePage

        ScheduleTypePage {

        }
    }
}
