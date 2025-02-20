import QtQuick

import Ronia

/*! ***********************************************************************************************
 * MonthDayDelegate
 * ***********************************************************************************************/
ItemDelegate {
    id: root

    /* Property Declaration
     * ****************************************************************************************/
    //! Day
    property var    dayModel

    /* Object Properties
     * ****************************************************************************************/
    text: dayModel.day

    contentItem: Label {
        text: parent.text
        color: enabled ? highlighted ? Style.background : Style.foreground : Style.hintTextColor
        verticalAlignment: "AlignVCenter"
        horizontalAlignment: "AlignHCenter"
    }


    background: Rectangle {
        implicitHeight: Style.delegateHeight

        radius: 8
        color: root.pressed ? Style.rippleColor : (root.highlighted ? Style.foreground : "transparent")
        border.width: dayModel.today && !root.highlighted ? 2 : 0
        border.color: Style.secondaryTextColor

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: root.enabled && root.hovered ? Style.rippleColor : "transparent"
        }
    }
}
