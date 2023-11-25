import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

import Ronia
import Ronia.impl

T.TextField {
    id: control

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    leftPadding: Style.textField.horizontalPadding
    rightPadding: leftPadding
    topPadding: Style.textField.verticalPadding
    bottomPadding: topPadding
    bottomInset: 16

    color: enabled ? Material.foreground : Material.hintTextColor
    selectionColor: Style.accent
    selectedTextColor: Style.background
    placeholderTextColor: Style.hintTextColor
    verticalAlignment: TextInput.AlignVCenter

    cursorDelegate: CursorDelegate { }

    PlaceholderText {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        elide: Text.ElideRight
        renderType: control.renderType
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
    }

    background: Item {
        implicitWidth: Style.textField.width
        implicitHeight: Style.textField.height

        Rectangle {
            y: parent.height - height
            width: parent.width
            height: control.activeFocus || (enabled && control.hovered) ? 2 : 1
            color: control.activeFocus ? Style.accent
                                       : ((enabled && control.hovered) ? Style.foreground
                                                                       : Style.hintTextColor)
        }
    }
}
