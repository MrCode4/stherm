import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * FanPage provide ui for changing fan settings
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to Fan model
    property Fan    fan

    /* Object properties
     * ****************************************************************************************/
    title: "Fan"

    /* Childrent
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Update Fan
            if (deviceController) {
                deviceController.updateFan();
            }
        }
    }

    //! Contents
    ColumnLayout {
        id: _contentsLay
        anchors.centerIn: parent
        spacing: 4

        ButtonGroup {
            buttons: [_autoButton, _onButton]
        }

        Button {
            id: _autoButton

            Material.theme: checked ? (_root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                    : _root.Material.theme
            Layout.alignment: Qt.AlignCenter
            leftPadding: 48
            rightPadding: 48
            font.weight: checked ? Font.ExtraBold : Font.Normal
            checked: true
            checkable: true
            text: "Auto"
        }

        Button {
            id: _onButton

            Material.theme: checked ? (_root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                    : _root.Material.theme
            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: _autoButton.width
            leftPadding: 48
            rightPadding: 48
            font.weight: checked ? Font.ExtraBold : Font.Normal
            checkable: true
            text: "On"
        }

        Label {
            Layout.topMargin: 32
            Layout.alignment: Qt.AlignCenter
            text: "Fan working period during each hour"
        }

        Slider {
            Layout.preferredWidth: _ticksRow.implicitWidth + implicitHandleWidth + leftPadding + rightPadding
            Layout.alignment: Qt.AlignCenter
            value: 0.5
            from: 0
            to: 55
            stepSize: 1
            snapMode: "SnapAlways"
            //! Uncomment following line to remove Slider's ticks (on backgroun)
            //! Component.onCompleted: background.children[0].repeater.model = 0
        }

        //! Slider ticks
        RowLayout {
            id: _ticksRow
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: -8
            spacing: 5

            Repeater {
                model: 56
                delegate: Rectangle {
                    Layout.alignment: Qt.AlignCenter
                    implicitWidth: 2
                    implicitHeight: (index % 5) ? 8 : 16
                    opacity: (index % 5) ? 0.5 : 1.
                    radius: 1
                }
            }
        }

        //! Ticks labels
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 0

            Repeater {
                model: 12
                delegate: Label {
                    Layout.preferredWidth: 5 * (_ticksRow.spacing + 2)
                    opacity: 0.5
                    text: index * 5
                    horizontalAlignment: "AlignHCenter"
                }
            }
        }
    }
}
