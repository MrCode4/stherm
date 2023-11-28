import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SensorLocationDelegate
 * ***********************************************************************************************/
ItemDelegate {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Location data
    property var location

    //! Index in model
    property var delegateIndex

    /* Object properties
     * ****************************************************************************************/
    font.pointSize: Application.font.pointSize * 0.9
    horizontalPadding: 0
    verticalPadding: 20
    checkable: true
    background: Rectangle {
        radius: 8
        color: root.checked ? Qt.darker(Style.foreground, 1.2) : "transparent"
        border.width: 1
        border.color: root.checked ? color : Style.foreground

        Rectangle {
            anchors {
                right: parent.right
                top: parent.top
                rightMargin: 4
                topMargin: 4
            }
            visible: root.checked
            color: Style.background
            width: 20
            height: 20
            radius: width / 2

            RoniaTextIcon {
                anchors.fill: parent
                anchors.margins: 3
                fontSizeMode: Text.Fit
                minimumPointSize: 8
                font.pointSize: Application.font.pointSize
                text: FAIcons.check
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 12

        Image {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            source: location?.icon ?? ""
            fillMode: Image.PreserveAspectFit
        }

        Label {
            Layout.fillWidth: true
            font.bold: root.checked
            text: location?.title ?? ""
            horizontalAlignment: "AlignHCenter"
            elide: Text.ElideRight
            color: root.checked ? Style.background : Style.foreground
        }
    }
}
