import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * FanPage provide ui for changing fan settings
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
            if (deviceController) {
                var nightMode = onButton.checked ? AppSpec.NMOn : AppSpec.NMOff
                deviceController.updateNightMode(nightMode);
            }

            //! Also move out of this Page
            if (root.StackView.view) {
                root.StackView.view.pop();
            }
        }
    }

    //! Contents
    ColumnLayout {
        id: contentsLay
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

    }

    Label {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: contentsLay.bottom
        anchors.topMargin: 10

        text: "The night mode will be active between 10:00 PM and 7:00 AM."

        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        visible: onButton.checked
    }
}
