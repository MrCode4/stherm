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
    property Fan    fan:    uiSession?.appModel?.fan

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

        TickedSlider {
            id: _hourSliders
            readonly property int tickStepSize: 2

            implicitWidth: implicitHeaderWidth * 3
            majorTickCount: ticksCount / 5
            ticksCount: to / tickStepSize
            from: 0
            to: 50
            stepSize: 1
            value: fan?.working_per_hour ?? 0

            ToolTip {
                parent: _hourSliders.handle
                y: -height - 16
                x: (parent.width - width) / 2
                visible: _hourSliders.pressed
                timeout: Number.MAX_VALUE
                delay: 0
                text: _hourSliders.value
            }

            onValueChanged: {
                if (fan && fan.working_per_hour !== value) {
                    fan.working_per_hour = value
                }
            }
        }
    }
}
