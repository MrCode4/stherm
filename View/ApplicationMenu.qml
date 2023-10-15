import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Stherm

/*! ***********************************************************************************************
 * ApplicationMenu provides ui to access different section of application
 * ***********************************************************************************************/
Drawer {
    id: _root

    /* Property declaration
     * ****************************************************************************************/
    //! Get a referenct to UiSession
    property UiSession      uiSession

    /* Object properties
     * ****************************************************************************************/
    width: parent.width
    height: parent.height
    topPadding: 0
    bottomPadding: 0
    modal: false //! \note: if this is true Drawer won't work due to existence of MouseArea in Main
    dim: true
    closePolicy: Popup.NoAutoClose
    contentItem: Page {
        header: RowLayout {
            ToolButton {
                contentItem: RoniaTextIcon {
                    color: _root.Material.foreground
                    text: "\uf060"
                }

                onClicked: _root.close();
            }

            Label {
                Layout.fillWidth: true
                textFormat: "MarkdownText"
                text: "## Menu"
                verticalAlignment: "AlignVCenter"
            }
        }

        contentItem: ApplicationMenuList {
            onMenuActivated: {
                //! Push related menu to stack
            }
        }
    }
}
