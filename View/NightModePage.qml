import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * NightMode provide ui for changing NightMode settings
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/
    //! Reference to nightMode model
    property NightMode        nightMode:    appModel?.nightMode ?? null

    /* Object properties
     * ****************************************************************************************/
    title: "Night Mode"

    /* Childrent
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Update NightMode
/

            //! Also move out of this Page
            if (root.StackView.view) {
                root.StackView.view.pop();
            }
        }
    }

    //! Contents
    ColumnLayout {
        id: _contentsLay
        anchors.centerIn: parent
        width: parent.width
        spacing: 8

        ButtonGroup {
            buttons: [onButton, offButton]
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 12

            Button {
                id: onButton

                Material.theme: checked ? (root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                        : root.Material.theme
                Layout.alignment: Qt.AlignCenter
                leftPadding: 64
                rightPadding: 64
                font.weight: checked ? Font.ExtraBold : Font.Normal
                checkable: true
                text: "On"

                checked: nightMode?.mode === AppSpec.NMOn
            }

            Button {
                id: offButton

                Material.theme: checked ? (root.Material.theme === Material.Dark ? Material.Light : Material.Dark)
                                        : root.Material.theme
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: onButton.width
                leftPadding: 64
                rightPadding: 64
                font.weight: checked ? Font.ExtraBold : Font.Normal
                checkable: true
                text: "Off"

                checked: nightMode?.mode === AppSpec.NMOff
            }
        }

        ColumnLayout {
            id: manualFanColumn
            opacity: 0
            enabled: false

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "Start Time"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        text: root.nightMode?.startTime ?? ""
                    }
                }

                onClicked: {
                    //! Open NightModeTimePage for editing
                    if (root.StackView.view) {
                        root.StackView.view.push("qrc:/Stherm/View/NightMode/NightModeTimePage.qml", {
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "timeProperty": "start-time",
                                                      "nightMode": root.nightMode
                                                  });
                    }
                }
            }

            ItemDelegate {
                Layout.fillWidth: true
                Layout.preferredHeight: Material.delegateHeight

                verticalPadding: 0
                horizontalPadding: 8
                contentItem: RowLayout {
                    spacing: 16

                    Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: "End Time"
                    }

                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: "AlignRight"
                        text: root.nightMode?.endTime ?? ""
                    }
                }

                onClicked: {
                    //! Open nightMode for editing
                    if (root.StackView.view) {
                        root.StackView.view.push("qrc:/Stherm/View/NightMode/NightModeTimePage.qml", {
                                                      "backButtonVisible": true,
                                                      "uiSession": uiSession,
                                                      "timeProperty": "end-time",
                                                      "nightMode": root.nightMode
                                                  });
                    }
                }
            }



            states: State {
                when: onButton.checked
                name: "visible"

                PropertyChanges {
                    target: manualFanColumn
                    opacity: 1
                    enabled: true
                }
            }

            transitions: Transition {
                to: "visible"
                reversible: true

                SequentialAnimation {
                    PropertyAnimation { target: manualFanColumn; property: "enabled"; duration: 0 }
                    NumberAnimation { property: "opacity" }
                }
            }
        }
    }
}
