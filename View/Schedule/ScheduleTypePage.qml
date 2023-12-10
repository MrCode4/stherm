import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleTypePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Schedule: If set changes are applied to it. This is can be used to edit a Schedule
    property Schedule           schedule

    //! Type is alwasy valid
    readonly property bool      isValid:    true

    //!
    readonly property string    type:       _buttonsGroup.checkedButton?.text ?? ""

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    topPadding: 24
    title: "Schedule Type"
    backButtonVisible: false
    titleHeadeingLevel: 4

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing and schedule (schedule is not null)
    ToolButton {
        parent: schedule ? _root.header.contentItem : _root
        visible: schedule
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            if (schedule && schedule.type !== _buttonsGroup.checkedButton.text) {
                schedule.type = _buttonsGroup.checkedButton.text;
            }

            backButtonCallback();
        }
    }

    ButtonGroup {
        id: _buttonsGroup
        buttons: _buttonsLay.children
    }

    ColumnLayout {
        id: _buttonsLay
        anchors.centerIn: parent
        width: parent.width * 0.4
        spacing: 12

        Button {
            Layout.fillWidth: true
            checkable: true
            checked: schedule ? schedule.type === text : true
            text: "Away"
        }

        Button {
            Layout.fillWidth: true
            checkable: true
            checked: schedule?.type === text
            text: "Night"
        }

        Button {
            Layout.fillWidth: true
            checkable: true
            checked: schedule?.type === text
            text: "Home"
        }

        Button {
            Layout.fillWidth: true
            checkable: true
            checked: schedule?.type === text
            text: "Custom"
        }
    }
}
