import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import QTemplateProject

/*! ***********************************************************************************************
 * Switch (2/3 way)
 * ************************************************************************************************/
Rectangle {
    id: control


    /* Property Declarations
     * ****************************************************************************************/
    //! Labels to be shown
    property var labels: ["First", "Second", "Third"]

    //! Selected index
    property int selectedIndex: 0


    /* Object Properties
     * ****************************************************************************************/
    implicitWidth: rowlayout.implicitWidth * 1.05
    implicitHeight: 38
    radius: 19
    color: AppStyle.backgroundGray
    opacity: enabled ? 1 : 0.35


    /* Children
     * ****************************************************************************************/
    Rectangle {
        id: indicatorRect
        width: 81
        height: 30
        radius: 15
        color: "black"
        border.color: AppStyle.primaryColor
        border.width: 2
        anchors.verticalCenter: parent.verticalCenter
        x: rowlayout.x + selectedIndex * rowlayout.implicitWidth / labels.length

        Behavior on x {
            NumberAnimation { easing.type: Easing.OutQuad;duration: 150}
        }
    }

    RowLayout {
        id: rowlayout
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        spacing: 0
        Repeater {
            model: labels
            delegate: Item {
                width: 81
                height: control.height
                Text {
                    color: selectedIndex === index ? AppStyle.primaryColor : "white"
                    font.family: "Roboto"
                    font.weight: selectedIndex === index ? 700 : 400
                    font.pixelSize: 18
                    text: modelData
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: control.selectedIndex = index
                }
            }
        }
    }




}
