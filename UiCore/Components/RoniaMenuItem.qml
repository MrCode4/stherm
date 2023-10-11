import QtQuick
import QtQuick.Controls

/*! ***********************************************************************************************
 * Customized Menu Item component to accepts textIcon
 * ************************************************************************************************/
MenuItem {
    id: control

    /* Property Declarations
     * ****************************************************************************************/
    property string iconCode

    /* Signals
     * ****************************************************************************************/

    /* Signal Handlers
     * ****************************************************************************************/

    /* Functions
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    contentItem: Item {
        RoniaTextIcon {
            id: textIcon
            color: control.enabled ? control.Material.foreground : control.Material.hintTextColor
            opacity: control.hovered ? 0.8 : 0.5
            text: control.iconCode
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
        }

        IconLabel {

            anchors.left: textIcon.right
            anchors.leftMargin: 15
            anchors.verticalCenter: textIcon.verticalCenter

            readonly property real arrowPadding: control.subMenu && control.arrow ? control.arrow.width + control.spacing : 0
            readonly property real indicatorPadding: control.checkable && control.indicator ? control.indicator.width + control.spacing : 0
            leftPadding: !control.mirrored ? indicatorPadding : arrowPadding
            rightPadding: control.mirrored ? indicatorPadding : arrowPadding

            spacing: control.spacing
            mirrored: control.mirrored
            display: control.display
            alignment: Qt.AlignLeft

            text: control.text
            font: control.font
            color: control.enabled ? control.Material.foreground : control.Material.hintTextColor
        }
    }
}
