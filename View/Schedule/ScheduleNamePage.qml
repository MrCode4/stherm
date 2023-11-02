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
    property alias  scheduleName: _nameTf.text

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
    }
}
