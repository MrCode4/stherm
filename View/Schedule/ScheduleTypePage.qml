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
    readonly property string type: _buttonsGroup.checkedButton?.text ?? ""

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: contentItem.children.length === 1 ? contentItem.children[0].implicitWidth + leftPadding + rightPadding : 0
    implicitHeight: contentItem.children.length === 1 ? contentItem.children[0].implicitHeight + implicitHeaderHeight
                                                        + implicitFooterHeight + topPadding + bottomPadding
                                                      : 0
    topPadding: 24
    title: "Schedule Type"
    backButtonVisible: false
    titleHeadeingLevel: 3

    /* Children
     * ****************************************************************************************/
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
            checked: true
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
