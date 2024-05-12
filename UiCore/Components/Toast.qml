import QtQuick
import Ronia

/*! ***********************************************************************************************
 * Toast: Simple UI element for displaying toast messages.
 * The name is based on a similar UI concept in the Android OS.
 * ************************************************************************************************/

//Root element which is a simple rounded and bordered rectangle
Rectangle {
    color: "gray"
    radius: 10
    border.width: 1
    border.color: "lightgray"
    opacity: 0 // Initially hidden

    property string message: "" // The toast message

    Text {
        anchors.centerIn: parent
        text: message
        color: "white"
        font: _fontMetric.font
    }

    FontMetrics {
        id: _fontMetric
        font.pointSize: font.pointSize * 0.9
    }

    // Makes the toast element visible and displays the current toast message
    function open(){
        opacity=1.0;
    }

    //Hides the toast element after the specified duration in ToastManager
    function close(){
        opacity=0.0;
    }

    //Applys a simple fade-in/fade-out animation to the toast component
    Behavior on opacity {
        NumberAnimation{duration: 1000}
    }
}
