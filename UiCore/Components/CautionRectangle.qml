import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * CautionRectangle
 * ***********************************************************************************************/
Rectangle {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    property string text: ""
    property string icon: FAIcons.triangleExclamation

    property color  iconColor: "#F59E0B"

    property real contentFontSize: Application.font.pointSize * 0.8

    /* Object properties
     * ****************************************************************************************/
    color: "#19140C"
    radius: 5

    //! In RichText, the implicitHeight could not detect correctly
    height: contenmtLayout.implicitHeight

    RowLayout {
        id: contenmtLayout
        Layout.alignment: Qt.AlignHCenter

        width: parent.width * 0.9
        spacing: 15

        RoniaTextIcon {
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.topMargin: 10
            Layout.leftMargin: 10

            text: icon
            color: iconColor
            font.pointSize: Application.font.pointSize * 0.8
        }

        Label {
            id: textLabel

            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 10
            Layout.bottomMargin: 5

            textFormat: Text.RichText
            text: root.text
            wrapMode: Text.WordWrap
            font.pointSize: contentFontSize

        }
    }
}
