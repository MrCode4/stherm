import QtQuick
import QtQuick.Layouts

import Ronia
import Stherm

/*! ***********************************************************************************************
 * SystemTypeHeatOnlyPage
 * ***********************************************************************************************/
BasePageView {
    id: _root

    /* Property declaration
     * ****************************************************************************************/

    /* Object properties
     * ****************************************************************************************/
    title: "Heat Only"

    /* Children
     * ****************************************************************************************/
    //! Confirm button
    ToolButton {
        parent: _root.header.contentItem
        contentItem: RoniaTextIcon {
            text: "\uf00c"
        }

        onClicked: {
            //! Do neccessary updates

            //! Also move out of this Page
            backButtonCallback();
        }
    }

    RowLayout {
        anchors.centerIn: parent
        spacing: 48

        Label {
            Layout.fillWidth: true
            text: "Heat Stages"
        }

        RowLayout {
            Layout.fillWidth: false

            RadioButton {
                checked: true
                text: "1"
            }

            RadioButton {
                text: "2"
            }

            RadioButton {
                text: "3"
            }
        }
    }
}
