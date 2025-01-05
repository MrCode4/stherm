import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * CountDownPopup: popup with count down timer to send emit startAction to parent
 * ***********************************************************************************************/
I_PopUp {
    id: root

    /* Property Declaration
     * ****************************************************************************************/
    property bool cancelEnable: true

    property int counterInSeconds: 5

    required property string actionText

    signal startAction();

    /* Object properties
     * ****************************************************************************************/
    title: " "
    titleBar: false

    width: AppStyle.size * 0.80
    closePolicy: Popup.NoAutoClose

    onVisibleChanged: {
        if (visible) {
            timer.counter = root.counterInSeconds;
            timer.start()
        }
        else {
            timer.stop()
        }
    }

    /* Children
     * ****************************************************************************************/

    Timer {
        id: timer
        property int counter: root.counterInSeconds

        repeat: true
        interval: 1000

        onTriggered: {
            timer.counter--;

            if (timer.counter <= 0) {
                startAction();
                stop();
            }
        }
    }

    ColumnLayout {
        id: mainLay

        width: parent?.width ?? 0
        anchors.centerIn: parent
        Layout.topMargin: 10
        spacing: 32

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pointSize: Application.font.pointSize * 1.5
            text: root.title
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.5
        }

        RoniaTextIcon {
            id: icon

            Layout.alignment: Qt.AlignHCenter
            font.pointSize: Style.fontIconSize.largePt * 1.5
            font.weight: 400
            text: FAIcons.restart
            visible: timer.counter < 1
        }

        Label {
            Layout.fillWidth: true
            font.pointSize: Application.font.pointSize * (timer.counter > 0 ? 1.5 : 0.9)
            text: timer.counter > 0 ? timer.counter : root.actionText
            horizontalAlignment: Text.AlignHCenter
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        ButtonInverted {

            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            visible: cancelEnable && timer.counter > 0
            text: qsTr("Cancel")

            onClicked: {
                close();
            }
        }
    }
}
