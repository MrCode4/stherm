import QtQuick 2.4

import MslGui

I_PopUp {
    id: item1
    width: 400
    height: 200

    Text {
        id: text1
        text: qsTr("Example PopUp")
        anchors.verticalCenter: parent.verticalCenter
        font.pixelSize: 50
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
