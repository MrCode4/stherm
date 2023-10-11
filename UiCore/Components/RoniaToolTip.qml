import QtQuick
import QtQuick.Controls
import QTemplateProject

ToolTip {
    id: control
    property color textColor: "white"

    text: qsTr("A descriptive tool tip of what the button does")
    width: 250
    font.weight: 600
    timeout: 3000
    delay: 500

    contentItem: Text {
        text: control.text
        font: control.font
        color: textColor
        opacity: 0.8
        wrapMode: Text.WordWrap
    }

    background: Rectangle {
        border.color: AppStyle.primaryColor
        border.width: 2
        color: AppStyle.inputBoxGray
        radius: 10
    }
}
