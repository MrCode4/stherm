import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleNamePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Signals
     * ****************************************************************************************/
    signal accepted()

    /* Property declaration
     * ****************************************************************************************/
    //! Schedule: If set changes are applied to Schedule. This is can be used to edit a Schedule
    property ScheduleCPP    schedule

    //! Whether name is valid or not
    readonly property bool  isValid:        _nameTf.acceptableInput

    //!
    property alias          scheduleName:   _nameTf.text

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: implicitHeaderHeight * 6 + _nameTf.implicitHeight + topPadding + bottomPadding
    title: "New Schedule Name"
    titleHeadeingLevel: 4
    backButtonVisible: false

    /* Children
     * ****************************************************************************************/
    //! Confirm button: only visible if is editing and schedule (schedule is not null)
    ToolButton {
        id: confirmBtn
        parent: schedule ? _root.header.contentItem : _root
        visible: schedule
        enabled: _nameTf.acceptableInput
        contentItem: RoniaTextIcon {
            text: FAIcons.check
        }

        onClicked: {
            if (schedule) {
                schedule.name = _nameTf.text;
            }

            backButtonCallback();
        }
    }

    TextField {
        id: _nameTf
        anchors.horizontalCenter: parent.horizontalCenter
        y: height * 0.8
        width: parent.width * 0.7
        placeholderText: "Enter Schedule Name"
        text: schedule?.name ?? ""
        validator: RegularExpressionValidator {
            regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
        }

        onAccepted: {
            if (confirmBtn.visible) {
                confirmBtn.forceActiveFocus();
                confirmBtn.clicked();
            }

            _root.accepted();
        }
    }

    Timer {
        id: delayedTextFieldFocusTmr
        running: true
        interval: 100
        onTriggered: {
            _nameTf.forceActiveFocus();
        }
    }
}
