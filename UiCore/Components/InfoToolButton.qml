import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * Info button
 * ***********************************************************************************************/

ToolButton {
    checkable: false
    checked: false
    implicitWidth: 64
    implicitHeight: implicitWidth
    icon.width: 50
    icon.height: 50

    contentItem: RoniaTextIcon {
        anchors.fill: parent
        font.pointSize: Style.fontIconSize.largePt
        Layout.alignment: Qt.AlignLeft
        text: FAIcons.circleInfo
    }
}
