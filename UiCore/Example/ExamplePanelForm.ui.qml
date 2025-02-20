import QtQuick 2.4
import QtQuick.Controls 2.4

import MslGui

I_Panel {
    id: examplePanel

    title: "Example Panel"

    width: 800
    height: 400
    property alias button2: button2
    property alias button1: button1
    property alias button: button

    Text {
        id: text1
        x: 0
        y: 0
        width: 800
        height: 65
        text: examplePanel.title
        font.pixelSize: 60
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Row {
        anchors.centerIn: parent
        spacing: 50

        // Question
        Button {
            id: button
            height: 200
            width: height
            text: "\uf128"
        }
        // Question
        Button {
            id: button1
            height: 200
            width: height
            text: "\uf128"
        }
        // Question
        Button {
            id: button2
            height: 200
            width: height
            text: "\uf128"
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:1}D{i:3}D{i:4}D{i:5}D{i:2}
}
##^##*/
