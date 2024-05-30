import QtQuick
import QtQuick.Layouts

import Ronia

/*! ***********************************************************************************************
 * Toast: Simple UI element for displaying toast messages.
 * The name is based on a similar UI concept in the Android OS.
 * ************************************************************************************************/

//Root element which is a simple rounded and bordered rectangle
Popup {
    id: root

    //* Property Declarations
    //* ************************************/

    //! The toast message
    property string message:    ""

    //! The detial of the message
    property string detail:     ""

    //* Object properties
    //* ************************************/
    horizontalPadding: 12
    font.pointSize: Application.font.pointSize * 0.9
    background: Rectangle {
        color: Style.disabledColor
        radius: height / 2
        border.width: 1
        border.color: Qt.lighter(color, 1.25)
    }
    leftMargin: 16
    rightMargin: leftMargin

    contentItem: GridLayout {
        property bool showDetailOnNextLine: {
            return detail.length > 0 ? metrics.advanceWidth(message + detail) > metrics.toastMaxWidth
                                     : false;
        }

        columns: showDetailOnNextLine ? 1 : 2

        //! Label for message
        Label {
            Layout.fillWidth: parent.showDetailOnNextLine
            text: message
            maximumLineCount: parent.showDetailOnNextLine ? 1 : 2
            wrapMode: parent.showDetailOnNextLine ? Text.NoWrap : "Wrap"
            elide: "ElideRight"
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
        }

        //! Label for detials
        Label {
            visible: detail.length > 0
            text: detail
            maximumLineCount: 2
            wrapMode: "Wrap"
            elide: "ElideRight"
            verticalAlignment: "AlignVCenter"
        }
    }

    FontMetrics {
        id: metrics

        readonly property real toastMaxWidth: root.ApplicationWindow.window ? root.ApplicationWindow.window.width - root.leftMargin - root.rightMargin : 0
    }
}
