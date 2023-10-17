import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ScheduleTypePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    readonly property string type: _buttonsGroup.checkedButton?.text ?? ""

    /* Object properties
     * ****************************************************************************************/
    title: "Schedule Type"
    backButtonVisible: false

    /* Children
     * ****************************************************************************************/
    ButtonGroup {
        id: _buttonsGroup
        buttons: _buttonsLay.children
    }

    ColumnLayout {
        id: _buttonsLay
        anchors.centerIn: parent

        Button {
            Layout.fillWidth: true
            checkable: true
            text: "Away"
        }

        Button {
            Layout.fillWidth: true
            checkable: true
            text: "Night"
        }

        Button {
            Layout.fillWidth: true
            checkable: true
            text: "Home"
        }

        Button {
            Layout.fillWidth: true
            checkable: true
            text: "Custom"
        }
    }
}
