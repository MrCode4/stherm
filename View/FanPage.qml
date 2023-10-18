import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * FanPage provide ui for changing fan settings
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to Fan model
    property Fan    fan : uiSession?.appModel?.fan

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
        spacing: AppStyle.size / 120

        ButtonGroup {
            buttons: [_autoButton, _onButton]
        }

        RowLayout {
            Layout.alignment: Qt.AlignCenter
            Button {
                id: _autoButton

                Material.theme: checked ? (_root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                        : _root.Material.theme
                Layout.alignment: Qt.AlignCenter
                leftPadding: AppStyle.size / 10
                rightPadding: AppStyle.size / 10
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
                leftPadding: AppStyle.size / 10
                rightPadding: AppStyle.size / 10
                font.weight: checked ? Font.ExtraBold : Font.Normal
                checkable: true
                text: "On"
            }
        }

        Label {
            Layout.topMargin: AppStyle.size / 15
            Layout.alignment: Qt.AlignCenter
            text: "Fan working period during each hour"
        }

        Slider {
            Layout.preferredWidth: _ticksRow.implicitWidth + implicitHandleWidth + leftPadding + rightPadding
            Layout.alignment: Qt.AlignCenter
            value: fan.working_per_hour
            from: 0
            to: 55
            stepSize: 1
            snapMode: "SnapAlways"
            //! Uncomment following line to remove Slider's ticks (on backgroun)
            //! Component.onCompleted: background.children[0].repeater.model = 0

            onValueChanged: fan.working_per_hour = value
        }

        //! Slider ticks
        RowLayout {
            id: _ticksRow
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: -8
            spacing: 6 * AppStyle.size / 240

            Repeater {
                model: 21
                delegate: Rectangle {
                    Layout.alignment: Qt.AlignCenter
                    implicitWidth: 2.5 * AppStyle.size / 240
                    implicitHeight: (index % 4) ? 8 : 16
                    opacity: (index % 4) ? 0.5 : 1.
                    radius: 2
                }
            }
        }

        //! Ticks labels
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 0

            Repeater {
                model: 6
                delegate: Label {
                    Layout.preferredWidth: 5 * (_ticksRow.spacing + 1)
                    opacity: 0.5
                    text: index * 10
                    horizontalAlignment: "AlignHCenter"
                }
            }
        }
    }
}
