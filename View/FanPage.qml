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
    property Fan    fan:    appModel?.fan ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Fan"

    /* Childrent
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Update Fan
            if (deviceController) {
                var fanMode = _autoButton.checked ? AppSpec.FanMode.FMAuto :
                                                    (_onButton.checked ? AppSpec.FanMode.FMOn :
                                                                         AppSpec.FanMode.FMOff)
                deviceController.updateFan(fanMode, _hourSliders.value);
            }

            //! Also move out of this Page
            if (_root.StackView.view && _root.StackView.view.depth > 1
                    && _root.StackView.view.currentItem === _root) {
                _root.StackView.view.pop();
            }
        }
    }

    //! Contents
    ColumnLayout {
        id: _contentsLay
        anchors.centerIn: parent
        width: parent.width
        spacing: 8 * scaleFactor

        ButtonGroup {
            buttons: [_autoButton, _onButton]
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 12

            Button {
                id: _autoButton

                Material.theme: checked ? (_root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                        : _root.Material.theme
                Layout.alignment: Qt.AlignCenter
                leftPadding: 64 * scaleFactor
                rightPadding: 64 * scaleFactor
                font.weight: checked ? Font.ExtraBold : Font.Normal
                checkable: true
                text: "Auto"

                checked: fan?.mode === AppSpec.FanMode.FMAuto
            }

            Button {
                id: _onButton

                Material.theme: checked ? (_root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                        : _root.Material.theme
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: _autoButton.width
                leftPadding: 64 * scaleFactor
                rightPadding: 64 * scaleFactor
                font.weight: checked ? Font.ExtraBold : Font.Normal
                checkable: true
                text: "On"

                checked: fan?.mode === AppSpec.FanMode.FMOn
            }
        }

        Label {
            id: _sliderDescLbl
            Layout.topMargin: 40 * scaleFactor
            Layout.fillWidth: true
            text: "Fan working period during each hour"
            wrapMode: "WrapAtWordBoundaryOrAnywhere"
            horizontalAlignment: "AlignHCenter"
        }

        RowLayout {
            spacing: 0

            Column {
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: _hourSliders.topPadding

                RoniaTextIcon {
                    x: (parent.width - width) / 2
                    font.pointSize: _root.font.pointSize * 0.8
                    text: FAIcons.clockThree
                }

                Label {
                    font.pointSize: _root.font.pointSize * 0.8
                    text: "min"
                }
            }

            TickedSlider {
                id: _hourSliders
                readonly property int tickStepSize: 2

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.rightMargin: 24 * scaleFactor
                majorTickCount: ticksCount / 5
                ticksCount: to / tickStepSize
                from: 0
                to: 50
                stepSize: 1
                value: fan?.workingPerHour ?? 0
                valueChangeAnimation: true

                ToolTip {
                    parent: _hourSliders.handle
                    y: -height - 16
                    x: (parent.width - width) / 2
                    visible: _hourSliders.pressed
                    timeout: Number.MAX_VALUE
                    delay: 0
                    text: _hourSliders.value
                }

                Label {
                    anchors.top: parent.bottom
                    x: 8 + parent.visualPosition * (parent.width - width - 16)
                    font.pointSize: _root.font.pointSize * 0.8
                    text: "min"
                }
            }
        }
    }
}
