import QtQuick
import QtQuick.Templates as T
import Qt5Compat.GraphicalEffects

Item {
    property alias text: textId.text
    property alias font: textId.font
    property alias color: textId.color
    property alias checkColor: colorOverlayId.color
    property alias checkBackground: checkIconId.color
    property int spacing: 4
    property T.Button button

    implicitWidth: textId.implicitWidth + (button?.checkable ? checkIconId.width : 0)
    implicitHeight: textId.implicitHeight

    Text {
        id: textId
        height: parent.height
        width: button?.checkable && button?.checked ? parent.width - checkIconId.width
                                                    : parent.width
        horizontalAlignment: Text.AlignHCenter
        clip: true
        elide: Text.ElideRight
    }

    Rectangle {
        id: checkIconId
        x: Math.max(textId.width + spacing, parent.width - width)
        y: (parent.height - height) / 2
        visible: button ? button.checkable && button.checked : false
        width: 16
        height: width
        radius: width / 2

        Image {
            id: iconImg
            anchors.fill: parent
            anchors.margins: 3
            visible: false
            source: "qrc:/Ronia/impl/res/check.png"
            sourceSize.width: width
            sourceSize.height: height
            fillMode: Image.PreserveAspectFit
        }

        ColorOverlay {
            id: colorOverlayId
            anchors.fill: iconImg
            source: iconImg
        }
    }
}
