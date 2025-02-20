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
    property ScheduleCPP        schedule

    //! Type is alwasy valid
    readonly property bool      isValid:    true

    //!
    readonly property int    type:       AppSpec.scheduleNameToType(_buttonsGroup.checkedButton.text)

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
            var type = AppSpec.scheduleNameToType(_buttonsGroup.checkedButton.text)

            if (schedule && schedule.type !== type) {
                schedule.type = type;
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

        Repeater {
            model: Object.values(AppSpec.scheduleTypeNames)

            delegate: Button {
                Layout.fillWidth: true
                checkable: true
                checked: modelData === AppSpec.scheduleTypeNames[schedule?.type ?? AppSpec.Away]
                text: modelData
            }
        }
    }
}
