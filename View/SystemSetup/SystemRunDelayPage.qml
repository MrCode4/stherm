import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemRunDelayPage provides ui for selecting run delay of system
 * ***********************************************************************************************/
BasePageView {
    id: root

    /* Property declaration
     * ****************************************************************************************/

    property int systemRunDelay: 1

    /* Object properties
     * ****************************************************************************************/
    title: "System Run Delay"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Apply settings
            if (deviceController) {
                deviceController.setSystemRunDelay(root.systemRunDelay)
            }

            //! pop this from StackView
            if (root.StackView.view) {
                root.StackView.view.pop()
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16

        Label {
            Layout.alignment: Qt.AlignCenter
            textFormat: "MarkdownText"
            text: "### Time"
        }

        RowLayout {
            RadioButton {
                text: "1 min"
                checked: appModel.systemSetup.systemRunDelay === 1

                onCheckedChanged: {
                    if (checked)
                        systemRunDelay = 1;
                }
            }
            RadioButton {
                text: "2 min"
                checked: appModel.systemSetup.systemRunDelay === 2

                onCheckedChanged: {
                    if (checked)
                        systemRunDelay = 2;
                }
            }
            RadioButton {
                text: "5 min"
                checked: appModel.systemSetup.systemRunDelay === 5

                onCheckedChanged: {
                    if (checked)
                        systemRunDelay = 5;
                }
            }
        }
    }
}
