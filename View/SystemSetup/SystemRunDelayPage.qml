import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemRunDelayPage provides ui for selecting run delay of system
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "System Run Delay"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header
        contentItem: RoniaTextIcon {
            text: "\uf00c" //! check icon
        }

        onClicked: {
            //! Apply settings and pop this from StackView
            //! Apply settings here

            if (_root.StackView.view) {
                _root.StackView.view.pop()
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
                id: _1minBtn
                text: "1 min"
                checked: true
            }
            RadioButton {
                id: _2minBtn
                text: "2 min"
            }
            RadioButton {
                id: _5minBtn
                text: "5 min"
            }
        }
    }
}
