import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorDelegate is a delegate to represent a sensor
 * ***********************************************************************************************/
Button {
    id: button

    /* Property declaration
     * ****************************************************************************************/
    //! Sensor
    property Sensor     sensor

    //! Delegate index in model/view
    property int        delegateIndex

    /* Object properties
     * ****************************************************************************************/
    implicitWidth: (mainLay.implicitWidth + (!checkIcon.visible ? checkIcon.implicitWidth + spacing: 0)) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             mainLay.implicitHeight + topPadding + bottomPadding)
    horizontalPadding: 16
    verticalPadding: 12
    text: sensor?.name ?? ""
    contentItem: Item {}

    /* Children
     * ****************************************************************************************/
    RowLayout {
        id: mainLay
        parent: button.contentItem
        anchors.fill: parent
        spacing: button.spacing

        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter
            color: enabled ? (checked ? Style.background : Style.foreground) : Style.hintTextColor
            font: button.font
            text: button.text
            elide: Text.ElideRight
            clip: true
            horizontalAlignment: Text.AlignHCenter
        }

        //! Some other infos about sensor, like signals, etc

        //! Checked icon
        Rectangle {
            id: checkIcon
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            visible: button.checkable && button.checked
            implicitWidth: 16
            implicitHeight: 16
            radius: width / 2
            color: Style.background

            Image {
                id: iconImg
                anchors.fill: parent
                anchors.margins: 3
                opacity: enabled ? 1. : 0.5
                source: "qrc:/Ronia/impl/res/check.png"
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
            }
        }
    }
}
