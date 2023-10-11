import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/*! ***********************************************************************************************
 * Flat button provides a standard flat button with Border/background and icon.
 *
 * \todo: we should extend the navigation button instead of simple Button
 * ************************************************************************************************/
Button {
    id: button

    /* Property Declarations
     * ****************************************************************************************/
    property color  checkedColor:   "white"
    property color  unCheckedColor: "white"
    property color  backColor: "transparent"
    property color  textColor: "white"
    property color  iconColor: textColor
    property color  hoverColor: Qt.darker(backColor, 1.03)
    property string description:    ""
    property alias  buttonLabel: buttonLabel
    property bool   round: false
    property color  borderColor: backColor == "transparent" ? "#42ADF5" : Qt.darker(backColor)
    property int    borderWidth: 1
    property real   radius: 3;
    property bool   solid: true
    property int    text_spacing: 10

    /* Object Properties
     * ****************************************************************************************/
    opacity: enabled ? 1 : 0.4

    font.family: "Font Awesome 6 Pro"
    font.weight: solid ? 900 : 400

    // background
    background: Rectangle {
        id: backRect
        implicitWidth: content.implicitWidth + 20    //! fill based on the content
        implicitHeight: content.implicitHeight + 20  //! fill based on the content
        radius: button.round ? height / 2 : button.radius
        border.color: borderColor
        smooth: true;
        color: button.checked ? checkedColor : ((button.hovered && enabled) ? hoverColor : backColor)
        border.width: borderWidth
    }

    /* Signal Handlers
     * ****************************************************************************************/

    /* Children
     * ****************************************************************************************/
    contentItem: Item {
        id: content
        RowLayout {
            id: rowLayout
            spacing: text_spacing
            anchors.centerIn: parent

            // Icon
            Text {
                id: icon
                text: button.text
                font: button.font
                color: iconColor
                visible: text === "" ? false : true
                opacity: 1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Layout.alignment: Qt.AlignCenter
            }

            // Button Text
            Text {
                id: buttonLabel;
                color: textColor
                opacity: 1
                Layout.fillWidth: true
                horizontalAlignment: icon.visible ? Text.AlignLeft : Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: text === "" ? false : true
                Layout.alignment: Qt.AlignLeft
            }
        }

        scale: (hovered && enabled) ? 1.03 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 50
            }
        }
    }

    ToolTip {
        visible: button.hovered && (description !== "")
        text: button.description
    }
}
