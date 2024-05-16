import QtQuick

import Ronia

/*! ***********************************************************************************************
 * Toast: Simple UI element for displaying toast messages.
 * The name is based on a similar UI concept in the Android OS.
 * ************************************************************************************************/

//Root element which is a simple rounded and bordered rectangle
Popup {
    property string message: "" // The toast message

    horizontalPadding: 12
    font.pointSize: Application.font.pointSize * 0.9
    background: Rectangle {
        color: Style.disabledColor
        radius: height / 2
        border.width: 1
        border.color: Qt.lighter(color, 1.25)
    }

    contentItem: Label {
        text: message
        maximumLineCount: 2
        wrapMode: "Wrap"
        elide: "ElideRight"
        verticalAlignment: "AlignVCenter"
        horizontalAlignment: "AlignHCenter"
    }
}
