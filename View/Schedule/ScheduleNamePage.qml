import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * ScheduleNamePage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Schedule: If set changes are applied to Schedule. This is can be used to edit a Schedule
    property Schedule       schedule

    //! Whether name is valid or not
    readonly property bool  isValid:        _nameTf.acceptableInput

    //!
    property alias          scheduleName:   _nameTf.text

    /* Object properties
     * ****************************************************************************************/
    implicitHeight: implicitHeaderHeight * 6 + _nameTf.implicitHeight + topPadding + bottomPadding
    title: "New Schedule Name"
    titleHeadeingLevel: 3
    backButtonVisible: false

    /* Children
     * ****************************************************************************************/
    TextField {
        id: _nameTf
        anchors.centerIn: parent
        width: parent.width * 0.7
        placeholderText: "Enter Schedule Name"
        text: schedule?.name ?? ""
        validator: RegularExpressionValidator {
            regularExpression: /^[^\s\\].*/ // At least 1 non-space characte
        }

        onEditingFinished: {
            if (schedule) {
                schedule.name = _nameTf.text;
            }
        }
    }
}
